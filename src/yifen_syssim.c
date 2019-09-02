/*做"memory cache"發生的事,(計算dirty ratio and 定時flush/hint,計算timer and 定量flush/hint,計算sum_block_count*/
/*應該要export給write buffer的:sum_block_count,定時hint,定量hint*/
/*141行跟822行有問題*/
/*
 * DiskSim Storage Subsystem Simulation Environment (Version 4.0)
 * Revision Authors: John Bucy, Greg Ganger
 * Contributors: John Griffin, Jiri Schindler, Steve Schlosser
 *
 * Copyright (c) of Carnegie Mellon University, 2001-2008.
 *
 * This software is being provided by the copyright holders under the
 * following license. By obtaining, using and/or copying this software,
 * you agree that you have read, understood, and will comply with the
 * following terms and conditions:
 *
 * Permission to reproduce, use, and prepare derivative works of this
 * software is granted provided the copyright and "No Warranty" statements
 * are included with all reproductions and derivative works and associated
 * documentation. This software may also be redistributed without charge
 * provided that the copyright and "No Warranty" statements are included
 * in all redistributions.
 *
 * NO WARRANTY. THIS SOFTWARE IS FURNISHED ON AN "AS IS" BASIS.
 * CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED AS TO THE MATTER INCLUDING, BUT NOT LIMITED
 * TO: WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY
 * OF RESULTS OR RESULTS OBTAINED FROM USE OF THIS SOFTWARE. CARNEGIE
 * MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH RESPECT
 * TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.
 * COPYRIGHT HOLDERS WILL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE
 * OR DOCUMENTATION.
 *
 */

/*
 * A sample skeleton for a system simulator that calls DiskSim as
 * a slave.
 *
 * Contributed by Eran Gabber of Lucent Technologies - Bell Laboratories
 *
 * Usage:
 *  syssim <parameters file> <output file> <max. block number>
 * Example:
 *  syssim parv.seagate out 2676846
 */

/*
one block=64 pages
*/

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"

#define NF 0
#define TF 1 
#define RP 2
#define FLUSHBACK 4
#define HINTGROUP 4

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include "syssim_driver.h"
#include "disksim_interface.h"
#include "disksim_rand48.h"
extern int sum_block_count[1000000] = {100}; //在my_ssd.c裡宣告
extern int clean_replace = 0;
extern int clean_flush1 = 0;
extern int clean_flush2 = 0;
extern long long int replace_time = 0;
extern long long int flush1_time = 0;
extern long long int flush2_time = 0;
extern int ssd_ARR = 0;
int flush_direct = 0 ;
int test_count2 = 0;
int Ref_temp = 0;
long long int Ref_diff = 0,diff = 0;
int Ref_dr = 0,dr = 0;
static float flush_count = 1024.0;
static float Mem =10.0;
static float dirty_ratio = 0.4;
static float dirty_count = 0.0;
static double Hitcount = 0.0,Sumcount = 0.0;
static int write_back_type; // 0->replacement 1->rule 1 2->rule 2 ( more than 30s)
static int old_dirty_time = 30 ; // 30
static int per_time = 5 ;
int read_miss = 0,read_hit = 0,write_miss = 0,write_hit = 0;
unsigned int clean2dirty=0;
double predict_time=0;
long long int evict_count = 0;
long long int count_to_buffer = 0; 
#define BLOCK 4096
#define SECTOR  512
#define BLOCK2SECTOR  (BLOCK/SECTOR)

typedef struct  {
  int n;
  double sum;
  double sqr;
} Stat;


SysTime now = 0;   /* current time */
static SysTime next_event = -1; /* next event */
static int completed = 0; /* last request was completed */
static Stat st;

// A Queue Node (Queue is implemented using Doubly Linked List)
typedef struct QNode
{
  struct QNode *prev, *next;
  long int blockNumber; // the page number stored in this QNode(±qtxtÅª¤Jªº) 
  unsigned Req_type; // 0 -> write , 1 -> read
  unsigned Dirty ; // 0 -> clean , 1 -> dirty
  unsigned write_type; // 0->replacement 1->rule 1 2->rule 2 ( more than 30s)
  unsigned size;
  unsigned devno;
  int block_count;
  long long int timestamp ;
} QNode;

// A Queue (A FIFO collection of Queue Nodes)
typedef struct Queue
{
  unsigned count; // Number of filled frames
  long long int numberOfFrames; // total number of frames
  QNode *front, *rear;
} Queue;

//Q:Queue指的是hint Queue還是cache Queue
//感覺是Cache Queue

// A hash (Collection of pointers to Queue Nodes)
typedef struct Hash
{
  int capacity; // how many pages can be there
  QNode **array; // an array of queue nodes
} Hash;

typedef struct Hint_page //all dirty page
{
  unsigned type; // NF/TF/RP
  unsigned page_num;
  double predict_time;
  struct Hint_page *prev, *next;
} Hint_page;

typedef struct Hint_Queue
{
  unsigned count; // number flush array offset
  long long int numberOfFrames; // total number of frames
  Hint_page *front, *rear;
} Hint_Queue;

Hint_Queue *global_Hint_Queue = NULL;

Hint_Queue* createHint_Queue( long long int numberOfFrames )//新增一個Queue(Queue的大小)
{
  Hint_Queue* queue = (Hint_Queue *)malloc( sizeof( Hint_Queue ) );//分配空間

  // The queue is empty
  queue->count = 0;
  queue->front = queue->rear = NULL;

  // Number of frames that can be stored in memory
  queue->numberOfFrames = numberOfFrames;
  return queue;
}

//blockNumberCount:計算目前"Queue中"每個Node各屬於哪個LBN,計算LBN目前在Queue持有幾個Node(sum_block_count[Blk])
void blockNumberCount(Queue *queue){
  QNode *block_count_Point = queue->rear;//block_count_Point=Queue的最後一個Node
  int i = 0;
  for(i = 0 ; i < 1000000 ; i++) //把sum_block_count清空
    sum_block_count[i] = NULL;

  while(1) {
    if(block_count_Point->Dirty == 1 ) {//如果最後一個Node是Dirty的話
      int Blk = block_count_Point->blockNumber / 64;//Blk=最後一個Node的blockNumber/block size 64 (屬於第幾個Logical BN)
      sum_block_count[Blk] ++;//sum_block_count[這個LBN]++(表示這個LBN目前上層持有dirty page個數)
    }
   // printf("blockNumber[%d] = %d\n" , block_count_Point->block_count,sum_block_count[block_count_Point->block_count]);
    //if(block_count_Point != NULL )printf("blkno = %d\n",block_count_Point->blockNumber );
    if(block_count_Point == queue->front || block_count_Point == NULL) break;//測完Queue中所有Node或者Queue是空的,break
    if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; //往前一個繼續測
    else break;
  }
  
}
//----------------------flush 1024 dirty pages---------------------
QNode *global_write_back_Point = NULL;
void write_back(Queue* queue , FILE *fwrite, struct disksim_interface *disksim, int on, double time ) { 
  printf("write_back|on=%d\n",on);
  int flush_Number = 0;
  int extra_flush_Number = 1;
  if(on == 1){ //定量flush
    QNode *current_Point = queue->rear;//目前的Node=最後的Node
    while(flush_Number < FLUSHBACK) {//目前flush page數量
      printf("in while\n");
      if(current_Point->Dirty == 1 ) {//目前的page是dirty,表示要flush
        printf("%d->Dirty=1\n",current_Point->blockNumber);
        current_Point->write_type = 0;//write_type=0表示??;read(1)write(0)
        struct disksim_request *r = malloc(sizeof(struct disksim_request));//動態配置disksim_request空間
        r->start = now;//現在時間
        r->flags = current_Point->write_type;//=0
        r->devno = current_Point->devno;//device number
        r->blkno = current_Point->blockNumber;//LPN
        r->bytecount = (current_Point->size) * 512;  // ssd 4096 //page size 4KB ??
        disksim_interface_request_arrive(disksim, now, r);//把r送進disksim裡

        /* Process events until this I/O is completed */
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
        }

       // fprintf(fwrite,"%ld\t%d\n",current_Point->blockNumber,current_Point->write_type);
        current_Point->Dirty = 0;//flush後變為clean
        //printf("flush_1 = %d\n",current_Point->blockNumber);
        dirty_count -= 1.0 ;//????
        flush_Number ++;//flush個數+1,若=1024則break
      
        if(current_Point != queue->front && current_Point != NULL) {//往前還有Node
          current_Point = current_Point->prev;
        }
        else if(current_Point == queue->front && current_Point->Dirty == 0 ) break;//遇到clean page跳過
        else if(current_Point == NULL) break;
      }
      else 
      {
        current_Point=current_Point->prev;
      }
    }
  }else{//on != 1 //hint flush也是定量flush但是write type=16 (extra_flush)
    QNode *current_Point = global_write_back_Point;//目前的Node=最後的Node
    clean_flush1 = 1;//????
    flush1_time = now;
    while(current_Point != queue->front && current_Point != NULL) {//目前page不是第一個
      if(current_Point->Dirty == 1) {//dirty page
        Hint_page *h = malloc(sizeof(struct Hint_page));
        h->page_num = current_Point->blkno/8;
        h->predict_time = time;
        if()
        //fprintf(fwrite,"%d\t%d\n",current_Point->blockNumber,current_Point->write_type);
        extra_flush_Number ++;//extra_flush次數+1 ???
      }
      if(extra_flush_Number == FLUSHBACK) 
      {
        current_Point->write_type = 16;//write_type=16(10)表示??
        struct disksim_request *r = malloc(sizeof(struct disksim_request));
        r->start = now;
        r->flags = current_Point->write_type;
        r->devno = current_Point->devno;
        r->blkno = 0;
        r->bytecount = 4096;  // ssd 4096
        disksim_interface_request_arrive(disksim, now, r);//把r送進disksim裡

        /* Process events until this I/O is completed */
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
        }
        break;//flush了1024個dirty page
      }
      else if (current_Point == queue->front) break;
      current_Point = current_Point->prev;
    }
 } 
 // blockNumberCount(queue);
}
QNode *global_flush_Point = NULL;
QNode *global_flush_MRU_Point = NULL;
QNode *global_flush_LRU_Point = NULL;
int record_flag = 0 ;
//----------------------flush 1024 location of pages -----------------------
//定時FLUSH
void period_write_back(Hash *hash , Queue* queue , FILE *fwrite , struct disksim_interface *disksim ,int on) {

  printf("period_write_back|on=%d\n",on);
  struct  timeval tv;
  long long int local_time ;
  gettimeofday(&tv,NULL);
  local_time = tv.tv_sec;
  int diff = 0 ;
  int tec;
  float i = 1.0;
  float extra_i = flush_count;
  int extra_flush_direct ;
  
  //printf("period_write_back\n");
  //printf("dirty_count=%f\n",dirty_count);
  QNode *current_Point2 ;

  //                 MRU            LRU
  //record_flag == 2   |<-----------
  //record_flag == 3    ----------->|
  if(record_flag == 0){
    current_Point2 = queue->rear;//預設從最後一個page找起
    if(on == 1)record_flag = 1;//第一輪??
  }else if(record_flag == 2){//第一輪刷完碰MRU的牆,從MRU開始找
     current_Point2 = queue->front;//2=指到第一個
     record_flag = 5;
  }else if(record_flag == 3){//第二輪刷完碰LRU的牆,從LRU開始找
     current_Point2 = queue->rear;//3=指到最後一個
     record_flag = 5;
  }else{//1,4,5=指到上次指的(往下一個或往上一個)
     current_Point2 = global_flush_Point;
  }
  if(global_flush_Point == NULL){
    global_flush_Point = queue->rear;//global_flush_Point=最後的page
  }
  //???
  if(global_flush_MRU_Point == queue->rear){
    current_Point2 = queue->front;
    global_flush_MRU_Point = NULL;
  }
  if(global_flush_LRU_Point == queue->front){
    current_Point2 = queue->rear;
    global_flush_LRU_Point = NULL;
  }
  if(on == 1){
    while(1){
      diff = now - ( (current_Point2)->timestamp + old_dirty_time );//現在時間-(候選page時間+30)>0 表示候選page待超過30秒
      //printf("now=%f\n",now);
      //printf("(current_Point2)->timestamp=%d\n",(current_Point2)->timestamp);
      //printf("diff=%d\n",diff);

      if(current_Point2->Dirty == 1 && diff >= 0) {

        struct disksim_request *r = malloc(sizeof(struct disksim_request));//刷下去
        current_Point2->write_type = 0;//write(0)
        r->start = now;
        r->flags = current_Point2->write_type;
        r->devno = current_Point2->devno;
        r->blkno = current_Point2->blockNumber;
        r->bytecount = (current_Point2->size) * 512;  // ssd 4096

        /* Process events until this I/O is completed */
        disksim_interface_request_arrive(disksim, now, r);
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
        }
      //  fprintf(fwrite,"%ld\t%d\n",current_Point2->blockNumber,current_Point2->write_type);
        current_Point2->Dirty = 0;
        dirty_count -=1.0;
        //printf("dirty_count=%f\n",dirty_count);
        global_flush_Point = current_Point2;//global_flush_Point=目前"測過"正刷下去的page
        if(i == flush_count || current_Point2 == NULL) {break;}//i=1024||
        if(flush_direct == 0){//MRU<-LRU
          if(current_Point2 != queue->front && current_Point2 != NULL){//目前的page在中間
            current_Point2 = current_Point2->prev;//point往前一個
          }else if (current_Point2 == queue->front){//目前page=第一個
            flush_direct = 1;//MRU<-LRU刷完一輪,改為MRU->LRU
            global_flush_MRU_Point = current_Point2;//MRUpoint=第一個page
            record_flag = 2;//第二輪
          }
        }
        if (flush_direct == 1){//MRU->LRU
          if(current_Point2 != queue->rear && current_Point2 != NULL){//還沒到底
            current_Point2 = current_Point2->next;//point往後一個
          }else if (current_Point2 == queue->rear){//刷到底了
            flush_direct = 0;//MRU->LRU刷完一輪,改為MRU<-LRU
            global_flush_LRU_Point = current_Point2;//LRUpoint=最後一個page
            record_flag = 3;//第三輪
          }
        }
      }else{ //clean or 沒有超過30秒
        global_flush_Point = current_Point2;//global_flush_Point=目前"測過"不用刷下去的page
        if(i == flush_count || current_Point2 == NULL) {break;}
        if(flush_direct == 0){//MRU<-LRU
          if(current_Point2 != queue->front && current_Point2 != NULL){
            current_Point2 = current_Point2->prev;
          }else if (current_Point2 == queue->front){
            flush_direct = 1;
            record_flag = 2;
          }
        }
        if (flush_direct == 1){//MRU->LRU
          if(current_Point2 != queue->rear && current_Point2 != NULL){
            current_Point2 = current_Point2->next;
          }else if (current_Point2 == queue->rear){
            flush_direct = 0;
            record_flag = 3;
          }
        }
      }
      i += 1.0;
    }//i=1024,break
  }else{ //on != 1 //hint flush 但是write type=16 (extra_flush)
    clean_flush2 = 1;
    flush2_time = now;
    extra_flush_direct = flush_direct ;//上次實際定時flush的方向(0/1)
    if(extra_flush_direct == 0 && current_Point2->prev != NULL){//MRU<-LRU && current_Point2前面有page
      current_Point2 = current_Point2->prev;//(global_flush_Point沒有動!!)
    }else if(extra_flush_direct == 1 && current_Point2->next != NULL){//MRU->LRU && current_Point2後面有page
      current_Point2 = current_Point2->next;
    }
    while(1) {
      diff = now - ( (current_Point2)->timestamp + old_dirty_time );
      if(current_Point2->Dirty == 1 && diff >= 0) {
        struct disksim_request *extra_r = malloc(sizeof(struct disksim_request));
        current_Point2->write_type = 16;//hint write
        extra_r->start = now;
        extra_r->flags = current_Point2->write_type;
        extra_r->devno = current_Point2->devno;
        extra_r->blkno = current_Point2->blockNumber;
        extra_r->bytecount = (current_Point2->size) * 512;  // ssd 4096
        disksim_interface_request_arrive(disksim, now, extra_r);

        /* Process events until this I/O is completed */
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
        }
      // fprintf(fwrite,"%ld\t%d\n",current_Point2->blockNumber,current_Point2->write_type);
        if(current_Point2 == queue->front || current_Point2 == queue->rear) break;//碰到底之後break//可能hint不到1024個
        if(extra_flush_direct == 0){//MRU<-LRU
          if(current_Point2 != queue->front && current_Point2 != NULL){
            current_Point2 = current_Point2->prev;
          }
        }else if (extra_flush_direct == 1){//MRU->LRU
          if(current_Point2 != queue->rear && current_Point2 != NULL){
            current_Point2 = current_Point2->next;
          }
        }
      }else{//clean or 沒有超過30秒
        if(current_Point2 == queue->front || current_Point2 == queue->rear) break;
        if(extra_flush_direct == 0){
          if(current_Point2 != queue->front && current_Point2 != NULL){
            current_Point2 = current_Point2->prev;
          }
        } 
        else if (extra_flush_direct == 1){
          if(current_Point2 != queue->rear && current_Point2 != NULL){
            current_Point2 = current_Point2->next;
          }
        }

      }
      if(extra_i == 1.0 || current_Point2 == NULL) break;//找到hint第1024個
      extra_i -= 1.0; //從1024往下減
    }
  }
 // blockNumberCount(queue);
}
// A utility function to create a new Queue Node. The queue Node
// will store the given 'blockNumber'
QNode *newQNode( unsigned blockNumber ) //新增一個Node(Node 的 blockNumber)
{
  // Allocate memory and assign 'blockNumber'
  QNode *temp = (QNode*)malloc( sizeof( QNode ) );//分配空間

  temp->blockNumber = blockNumber;
  // Initialize prev and next as NULL
  temp->prev = NULL;
  temp->next = NULL;
  return temp;
}

// A utility function to create an empty Queue.
// The queue can have at most 'numberOfFrames' nodes
Queue* createQueue( long long int numberOfFrames )//新增一個Queue(Queue的大小)
{
  Queue* queue = (Queue *)malloc( sizeof( Queue ) );//分配空間

  // The queue is empty
  queue->count = 0;
  queue->front = queue->rear = NULL;

  // Number of frames that can be stored in memory
  queue->numberOfFrames = numberOfFrames;
  return queue;
}

// A utility function to create an empty Hash of given capacity
Hash* createHash( long long int capacity )
{
  // Allocate memory for hash
  Hash *hash = (Hash *) malloc( sizeof( Hash ) );
  hash->capacity = capacity;

  // Create an array of pointers for refering queue nodes
  hash->array = (QNode **) malloc( hash->capacity * sizeof( QNode* ) );

  // Initialize all hash entries as empty
  int i;
  for( i = 0; i < hash->capacity; ++i )
    hash->array[i] = NULL;
  return hash;
}

// A function to check if there is slot available in memory
int AreAllHintsFull( Hint_Queue* queue )
{
  return queue->count == queue->numberOfFrames; 
}

// A function to check if there is slot available in memory
int AreAllFramesFull( Queue* queue )
{
  return queue->count == queue->numberOfFrames; 
}

// A utility function to check if queue is empty
int isQueueEmpty( Queue* queue )
{
  return queue->rear == NULL;
}

//Hash 放cache裡的東西(=Queue) 為了搜尋快速 hash->array[block number]=QNode
//replacement(queue放page cache的page,hash,fwrite寫入檔案?,incoming_blockNumber新進來的的page的bn,disksim_interface)
void replacement(Queue* queue , Hash* hash , FILE *fwrite , unsigned incoming_blockNumber ,struct disksim_interface *disksim ){
  QNode *current_Point2 = queue->rear;//current_Point2初始指到LRU端
  QNode *current_Point = queue->rear;//current_Point初始指到LRU端
  write_back_type == 0;
  QNode *reqPage = hash->array[ incoming_blockNumber ];//找到array裡bn的QNode存進reqPage
  int extra_no = 0;
//  printf("incoming page = %d\n", incoming_blockNumber);
  printf("replacement\nincoming page = %d\n", incoming_blockNumber);
  //LRU端Page=Dirty && LRU端Page不是新進page && hash->array[ incoming_blockNumber ]=null(目前ssd裡沒有??)
  if(current_Point2->Dirty == 1 && current_Point2->blockNumber != incoming_blockNumber && reqPage == NULL ) {
      struct disksim_request *r = malloc(sizeof(struct disksim_request));
        current_Point2->write_type = 0;//直接寫入LRU端Page,讓LRU端Page變成clean
        r->start = now;
        r->flags = current_Point2->write_type;
        r->devno = current_Point2->devno;
        r->blkno = current_Point2->blockNumber;
        r->bytecount = (current_Point2->size) * 512;  // ssd 4096
        current_Point2->Dirty = 0;
        dirty_count -=1.0;
        disksim_interface_request_arrive(disksim, now, r);
        while(next_event >= 0) {
          now = next_event;
          next_event = -1;
          disksim_interface_internal_event(disksim, now, 0);
        }         
  }
  clean_replace = 1;
  replace_time = now; //全域變數,這時發生replacement
  //目前LRU page有東西 && extra_no=0
  while(current_Point != NULL && extra_no < 1) {
    if(current_Point->Dirty == 1) { //目前還是dirty 沒有被上面那個replace掉
        current_Point->write_type = 8; //write_type = 8
        struct disksim_request *r = malloc(sizeof(struct disksim_request));
        r->start = now;
        r->flags = current_Point->write_type;
        r->devno = current_Point->devno;
        r->blkno = current_Point->blockNumber;
        r->bytecount = (current_Point->size) * 512;  // ssd 4096
        disksim_interface_request_arrive(disksim, now, r);
        while(next_event >= 0) {
        now = next_event;
        next_event = -1;
        disksim_interface_internal_event(disksim, now, 0);
      }
      extra_no ++;
    }
    current_Point = current_Point->prev;
  }
  
}
// A utility function to "delete a frame" from queue
void deQueue( Queue* queue ){
  printf("deQueue\n");
  QNode *current_Point = queue->rear;//current_Point初始指到LRU端
  if( isQueueEmpty( queue ) )
    return;
  if (queue->front == queue->rear) queue->front = NULL; // If this is the only node in list, then change front
 // printf("real_replacement page = %d\n", current_Point->blockNumber);
  queue->rear = queue->rear->prev;//1個node,queue->rear=null//2個node,queue->rear= queue->rear->prev
  
  
  if (queue->rear)//2個node的情況
    queue->rear->next = NULL;//刪掉第2個node

  // decrement the number of full frames by 1
  queue->count--;//queue大小-1
  free(current_Point);//釋放空間

}

// A function to add a page with given 'blockNumber' to both queue
// and "hash" replacement
//加入最新req,只有更改queue的指標,沒有做寫入
void Enqueue( Queue* queue, Hash* hash, unsigned blockNumber )
{
  //printf("Enqueue\n");
  // If all frames are full, remove the page at the rear
  if ( AreAllFramesFull ( queue ) ){
    hash->array[ queue->rear->blockNumber ] = NULL;// remove page from hash
    deQueue( queue );
  }

  // Create a new node with given page number,
  // And add the new node to the front of queue
  // newQNode¬O²£¥Í¤@­Ó¿W¥ßªº·s¸`ÂI¡A©Ò¥H¤@¶}©l¥LªºÀY§À³£¨SªF¦è
  // §â­ì¥»¦bMRUºÝªºpage²¾°Ê¨ìblnumªº¤U¤@­Ó
  // ¤@¶}©lªì©l¤Æ®É¡Aprev¬ONULL
  QNode *temp = newQNode( blockNumber  );//新node
  temp->next = queue->front;//把temp加到queue的最前面

  // If queue is empty, change both front and rear pointers
  //¦pªG¤@¶}©l¬OªÅªº¡A¥Ø«e­n·s¼W¤@­Ónode¶i¨Ó¡A¨ºquequ¸Ì­±ªºMRU©MLRU³£¬O¦P¤@­Ó 
  if ( isQueueEmpty( queue ) )
    queue->rear = queue->front = temp;
  else // Else change the front
  {
    queue->front->prev = temp; //µ¥©ó¬O¸ò blnum->next = queue->front; ¹ïÀ³ 
    queue->front = temp;       //©Ò¥H¦¹®Éblnum³o­Ó·sªºpage´NÅÜ¦¨MRU 
  }

  // Add page entry to hash also
  hash->array[ blockNumber ] = temp;//在hash的blocknumber位置也加上node
  
  // increment number of full frames
  queue->count++;//queue大小增加

}

// This function is called when a page with given 'blockNumber' is referenced
// from cache (or memory). There are two cases:
// 1. Frame is not there in memory, we bring it in memory and add to the front
// of queue
// 2. Frame is there in memory, we move the frame to front of queue
void ReferencePage( Queue* queue, Hash* hash, double Req_time, long int Req_devno, long int blockNumber, long int Req_size, long int Req_type, struct disksim_interface *disksim,FILE *fwrite){
  struct  timeval    tv;
  long long int local_time;
  gettimeofday(&tv,NULL);
  local_time = tv.tv_sec;
  printf("ReferencePage\n");
  int i,j;
 
  //printf("diff=%d\n",diff);
  //printf("ReferencePage\n");
  QNode *reqPage = hash->array[ blockNumber ]; //有沒有在記憶體裡
  struct disksim_request *r = malloc(sizeof(struct disksim_request));
  // the page is not in cache, bring it
  if ( reqPage == NULL ) 
  { //miss
    if(Req_type==1)//read
    {
      printf("read req\n");
      r->start = now;
      r->flags = Req_type;
      r->devno = Req_devno;
      r->blkno = blockNumber;
      r->bytecount = Req_size * 512;  // ssd 4096
      //printf("last\n");
      //去讀或寫page的時間
      disksim_interface_request_arrive(disksim, now, r);
      while(next_event >= 0) {
        now = next_event;
        next_event = -1;
        disksim_interface_internal_event(disksim, now, 0);
      }
    }
    Enqueue( queue, hash, blockNumber);//更改Queue指標


    QNode *reqPage = hash->array[ blockNumber ]; //記錄進hash
    reqPage->Req_type = Req_type;
    reqPage->size = Req_size;
    reqPage->devno = Req_devno;
    reqPage->timestamp = now;
    reqPage->block_count = (blockNumber/ 64 ) ;//didn't used
    if (reqPage->Req_type == 1 ){ //read
      reqPage->Dirty = 0;
    }else{ //write
      clean2dirty++;
      reqPage->Dirty = 1;
      dirty_count += 1.0;//40%的那個
    }
  } 
  else if (reqPage == queue->front)
  { // hit!!,page is there and at front, change pointer
    if(Req_type == 0)
    {   //新進來的是write
      if(reqPage->Req_type == 1 )
      {  //原本的是read
        clean2dirty++;
        reqPage->Req_type = Req_type;  //更新type
        reqPage->Dirty = 1; //設為dirty
        dirty_count += 1.0; //40%的+1
      }
      else
      { //原本的是write
        reqPage->Req_type = Req_type; 
        if(reqPage->Dirty != 1)
        { //原本是clean(because flush),因為新的是write所以改成dirty
          clean2dirty++;
          reqPage->Dirty = 1; 
          dirty_count += 1.0; 
        }
      }       
    } 
    Hitcount++;// hit!!
  }
  else if (reqPage != queue->front)
  { // hit!!page is there and not at front, change pointer
    //做跟上面一樣的事
    if(Req_type == 0){  //if type of new request is 0(write) , update type of the new reqest , //and dirty always is true;
      if(reqPage->Req_type == 1 ){  
        clean2dirty++;
        reqPage->Req_type = Req_type;  
        reqPage->Dirty = 1;
        dirty_count += 1.0;  // 0->1 ,so dirty count ++
      }else{
        reqPage->Req_type = Req_type;  
        if(reqPage->Dirty != 1){
          clean2dirty++;
          reqPage->Dirty = 1;
          dirty_count += 1.0;
        }
      }       
    }
    Hitcount++;
    // Unlink rquested page from its current location
    // in queue.
    //比上面多出來的,要改Queue的point,拔出這個Node
    reqPage->prev->next = reqPage->next;
    if (reqPage->next)
      reqPage->next->prev = reqPage->prev;

    // If the requested page is rear, then change rear
    // as this node will be moved to front
    //如果是最LRU端的話還要再改Queue的rear指標
    if (reqPage == queue->rear){
      queue->rear = reqPage->prev;
      queue->rear->next = NULL;
    }

    // Put the requested page before current front
    //把拔出來的Node插到最前面
    reqPage->next = queue->front;
    reqPage->prev = NULL;

    // Change prev of current front
    reqPage->next->prev = reqPage;

    // Change front to the requested page
    queue->front = reqPage;
  }
  //計算什麼時候要下hint (39%dirty or timer=5sec)
  if(Req_type == 0 ){//write 更改待在 queue裡的時間
    if(diff == 0) Ref_diff = now + 4 ; //全域diff=現在時間-(候選page待在memory時間+30),預設diff=0
    else Ref_diff = diff -1;
    Ref_temp = 1 ;
    Ref_dr = now - Ref_diff;
    if( Ref_dr >=0 ){
      period_write_back(hash, queue ,fwrite,disksim,0);//定時flush hint
    }
    if((dirty_count/Mem) >= (dirty_ratio - 0.01)) { //定量flush hint
      pre_time=now;
      int hint_g=0;
      HintQ = createHint_Queue(8192);//16 page
      global_write_back_Point = queue->rear;
      write_back(HintQ, queue ,fwrite,disksim,0,now);
      hint_g++;
      while(1)
      {
        if(hint_g==HINTGROUP)break;
        next_Nflush_time=(pre_time/clean2dirty)*(clean2dirty+1024);
        Nflush_time_diff=pre_time- next_Nflush_time;
        next_Tflush_time=((int)(pre_time/5)+1)*5;
        if(next_Nflush_time>=next_Tflush_time)
        {
          //Tflush
          for(i=0;i<(Nflush_time_diff - (next_Tflush_time - pre_time))/5 ; i++)
          {
            if(hint_g==HINTGROUP)continue;
            //Tflush
            next_Tflush_time=next_Tflush_time+5;

            hint_g++;
          }
          if(hint_g==HINTGROUP)break;
          write_back(HintQ, queue, fwrite,disksim,0,next_Nflush_time);
          hint_g++;
          pre_time=next_Nflush_time;
          next_Tflush_time=next_Tflush_time+5;
          //period_write_back)//first do period write back
        }
        else if(next_Nflush_time<next_Tflush_time)
        {
          write_back(HintQ, queue, fwrite,disksim,0,next_Nflush_time);
        }
      }
      global_Hint_Queue=HintQ;
      free(HintQ);
      
      write_back(queue ,fwrite,disksim,0,next_Nflush_time);
    }
  }
  printf("clean2dirty=%d\n",clean2dirty);

  //printf("diff=%d\n",diff);
}




void
panic(const char *s)
{
  perror(s);
  exit(1);
}


void
add_statistics(Stat *s, double x)
{
  s->n++;
  s->sum += x;
  s->sqr += x*x;
}


void
print_statistics(Stat *s, const char *title)
{
  double avg, std;

  avg = s->sum/s->n;
  std = sqrt((s->sqr - 2*avg*s->sum + s->n*avg*avg) / s->n);
  printf("%s: n=%d average=%f std. deviation=%f\n", title, s->n, avg, std);
  printf("read miss = %d\n", read_miss);
  printf("read hit = %d\n", read_hit);
  printf("write miss = %d\n", write_miss);
  printf("write hit = %d\n", write_hit);
  printf("all count = %lld\n",count_to_buffer);
  printf("evict_count = %lld\n", evict_count);
}


/*
 * Schedule next callback at time t.
 * Note that there is only *one* outstanding callback at any given time.
 * The callback is for the earliest event.
 */
void
syssim_schedule_callback(disksim_interface_callback_t fn, 
       SysTime t, 
       void *ctx)
{
  next_event = t;
}


/*
 * de-scehdule a callback.
 */
void
syssim_deschedule_callback(double t, void *ctx)
{
  next_event = -1;
}


void
syssim_report_completion(SysTime t, struct disksim_request *r, void *ctx)
{
  completed = 1;
  now = t;
  add_statistics(&st, t - r->start);
}


int main(int argc, char *argv[]) {
  int i;
  int nsectors = 1000;
  struct stat buf;
  struct disksim_request r;
  struct disksim_interface *disksim;

  struct  timeval    tv;
  long long int Global_time ;
  int temp = 0;
  double time;
  int devno;
  int blnum=0,R_W=0,size;
  int temp_2 = 0 ;
  
  float evict_time;
  long int evict_devno , evict_blnum , evict_size , evict_flags;
  //printf("tv_sec:%ld\n",Global_time);
  printf("<<<ssd_ARR=%d>>\n",ssd_ARR);
  printf("tv_sec:%ld\n",Global_time);
  FILE *fwrite = fopen("/home/osnet/disksim_blas_Yifen/src/flush","w");
  FILE *fread = fopen("/home/osnet/disksim_blas_Yifen/src/test5.txt","r");
  //FILE *evict_fread = fopen("/home/osnet/disksim_blas_Yifen/src/BPLRU/iozone/2048","r");

  Queue *q = createQueue( Mem ); // Let cache can hold x pages (Mem全域)
  Hash *hash = createHash( 100000000 );// Let 10 different pages can be requested (pages to be referenced are numbered from 0 to 9 )
///////////原本就有↓////////////////
  if (argc != 4 ) {
    fprintf(stderr, "usage: %s <param file> <output file> <#sectors>\n",argv[0]);
    exit(1);
  }

  if (stat(argv[1], &buf) < 0)
    panic(argv[1]);

  disksim = disksim_interface_initialize(argv[1], 
           argv[2],
           syssim_report_completion,
           syssim_schedule_callback,
           syssim_deschedule_callback,
           0,
           0,
           0);
/////////原本就有↑////////////////

  if (fread == NULL)
    perror ("Error opening file");
  else {
    while(fscanf(fread,"%lf%ld%ld%ld%ld",&time,&devno,&blnum,&size,&R_W)!= EOF){ //接收新的request
      printf("----接收新的request----\n");
      // printf("%f\n",dirty_count );
      //printf("dirty_count=%f\n",dirty_count );
      Sumcount++; 
      test_count2 ++ ;
      if(temp == 0){ //第一次讀req
        // printf("----(temp == 0)接收新的request----\n");
        //printf("temp==0\n");
        diff = now + per_time ; //diff=now+5
        //printf("now=%f,diff=%d\n",now,diff);
        temp = 1 ;
      }
      if ( AreAllFramesFull ( q ) ){ //memory是滿的(Queue是滿的)
          replacement(q , hash, fwrite , blnum , disksim); //新的req進來要做replace
      }
      ReferencePage( q, hash, time, devno, blnum, size, R_W, disksim,fwrite); //把新的req放到MRU
      //printf("----把新的req放到MRU----\n");
      QNode *block_count_Point = q->rear; //取LRU的Node,從最後往前算block_count
      int i = 0;
      for(i = 0 ; i < 1000000 ; i++) sum_block_count[i] = 0; //整個ssd有1000000個block,sum_block_count[]歸零
      while(1) {
        if(block_count_Point->Dirty == 1 ) { //選定的Node是dirty
          int Blk = ((block_count_Point->blockNumber/8) / 64 ) ;//為什麼要除以8!!!!!
          sum_block_count[Blk] ++;
        }
        if(block_count_Point == q->front || block_count_Point == NULL) break;
        if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; //往前一個繼續算
        else break;
      }
      if((dirty_count/Mem) >= dirty_ratio){ //定量flush, flush_1 number of dirty more than dirty ratio , exe flush 
        // printf("----實際執行定量flush----\n");

        write_back(q ,fwrite,disksim,1,0); //實際執行定量flush
        //重算sum_block_count↓///
        QNode *block_count_Point = q->rear;
        int i = 0;
        for(i = 0 ; i < 1000000 ; i++) sum_block_count[i] = 0;
        while(1) {
          if(block_count_Point->Dirty == 1 ) {
            int Blk = ((block_count_Point->blockNumber/8) / 64 ) ;
            sum_block_count[Blk] ++;
          }
          if(block_count_Point == q->front || block_count_Point == NULL) break;
          if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; 
          else break;
        }
      }
          
      dr = now - diff;//算時間的QQ 
      //printf("diff=%f\n",diff);
      //diff maybe (last exe timer flush) + 5 
      //ex.now=25,last flush=20,diff=20+5=25,now-diff>=0,next timer flish
      //printf("test_count2 = %d\n", test_count2 );
      //printf("test_count2 = %d\n", test_count2 );
      //printf("dr = %d \n", dr );
      if( dr >= 0) { // per 5 second to wake up 
        temp = 0; //當再進一個新的req時重新計算時間
        //printf("dirty_count=%f\n",dirty_count);
        //sleep(10);
        //printf("----實際執行定時flush----\n");
        period_write_back(hash, q,fwrite,disksim,1); //實際執行定時flush
        //sleep(10);
        //重算sum_block_count↓///
        QNode *block_count_Point = q->rear;
        int i = 0;
        for(i = 0 ; i < 1000000 ; i++) sum_block_count[i] = 0;
        while(1) {
          if(block_count_Point->Dirty == 1 ) {
            int Blk = ((block_count_Point->blockNumber/8) / 64 ) ;
            sum_block_count[Blk] ++;
          }
          if(block_count_Point == q->front || block_count_Point == NULL) break;
          if(block_count_Point->prev != NULL) block_count_Point = block_count_Point->prev; 
          else break;
        }
      }
      printf("Queue:");
      QNode *p = q->front;
      while(1)
      {
        printf("[%d|d=%d]",p->blockNumber,p->Dirty);
        if(p->next==NULL)break;
        else p=p->next;
      } 
      printf("\n");
    }
        
  }
  int kk = 0;
  printf("kk = %d \n", kk);
  // Let us print cache frames after the above referenced pages
/*
 while(fscanf(evict_fread,"%f%ld%ld%ld%ld",&evict_time,&evict_devno,&evict_blnum,&evict_size,&evict_flags)!= EOF){
  //kk++;
  //printf("kk = %d \n", kk);
  struct disksim_request *r = malloc(sizeof(struct disksim_request));
  // the page is not in cache, bring it
    r->start = evict_time;
    r->flags = evict_flags;
    r->devno = evict_devno;
    r->blkno = evict_blnum;
    r->bytecount = evict_size;  
    disksim_interface_request_arrive(disksim, evict_time, r);
    while(next_event >= 0) {
      now = next_event;
      next_event = -1;
      disksim_interface_internal_event(disksim, now, 0);
    }
  }*/
  int j=0;
 
  /*while(j<=1000){
    QNode *cur = hash->array[j];
    if(cur!= NULL)
     printf("blkno = %d , invalid = %d\n",cur->blockNumber ,cur->invalid);
     cur = cur->next;
    j++;
  }*/
 /* while(print_cur != NULL){
        printf ("%d\n", print_cur->F_blockNumber);
        print_cur = print_cur->F_next;
      }*/
 // printf ("invalid =%d\n", cur->invalid);
  printf("<<<ssd_ARR=%d>>>\n",ssd_ARR);
  printf("Sumcount = %.0lf,Hitcount = %.0lf Hit = %.2f\n",Sumcount,Hitcount,(Hitcount/Sumcount)*100.0);
  
//-----------------------------------------------------------------------------------------------------------------------
 

  /* NOTE: it is bad to use this internal disksim call from external... */
  DISKSIM_srand48(1);

 /* for (i=0; i < 1; i++) {
    r.start = now;
    r.flags = DISKSIM_READ;
    r.devno = 0;

    // NOTE: it is bad to use this internal disksim call from external... 
    r.blkno = BLOCK2SECTOR*(DISKSIM_lrand48()%(nsectors/BLOCK2SECTOR));
    r.bytecount = BLOCK;
    completed = 0;
    disksim_interface_request_arrive(disksim, now, &r);

    // Process events until this I/O is completed 
    

    if (!completed) {
      fprintf(stderr,
        "%s: internal error. Last event not completed %d\n",
        argv[0], i);
      exit(1);
    }
  }*/
 
  disksim_interface_shutdown(disksim, now);
  print_statistics(&st, "response time");
  //fclose(evict_fread);
  fclose(fread);
  fclose(fwrite);
  exit(0);
}
