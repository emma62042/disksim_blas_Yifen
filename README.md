disksim_blas_Yifen
===

## Table of Contents
- [disksim_blas_Yifen](#disksim-blas-yifen)
  * [Beginners Guide](#beginners-guide)
  * [如何執行](#如何執行)
  * [執行結果](#執行結果)

## Beginners Guide

If you are a total beginner to this, start here!

A Host-hinted Write Buffer Management and Striping Scheme for SSDs
利用主機端資訊管理固態硬碟寫入緩衝區與資料擺放的方法


## 如何執行

**Make**  
```cmd
    # The first example has two steps
    # 只要有修改裡面的檔案都要make,make的順序是1.ssdmodel 2.src
    # 例如修改了/ssdmodel/ssd.c 就要在ssdmodel make, 接著修改src中任一檔案再make src
    /ssdmodel: make
    /src: make
```
  
**執行程式**
```cmd
    # 在根目錄執行
    # 參數設定 ./src/syssim <param file> <output file> <#sectors> <input trace file> <max_req> <memory_size(#pages)>
    # <max_req> : 如果要計算後1/3的平均req responsetime,設定這個trace送下去ssd的max request個數,若計算全部的response time則設成1即可
    # <memory_size(#pages)> : page cache可容納page數
    /: ./src/syssim ./ssdmodel/valid/ssd-iozone.parv ./outpup.txt 2676846 /home/osnet/SNIA_trace/FIU/User2/run1_asim_usr20.txt 1 6488
```
**define參數調整**
```cmd
    /ssdmodel/ssd.c :
    "RWRATIO"     : write buffer eviction window 當中每個'block'的read count/read count+write count > 這個值,就page striping
                    <例> 論文中的write ratio=0.6就是"RWRATIO"設成0.4
    "EVICTWINDOW" : eviction window 的block數
                    <例> 論文中的eviction window=1/2就是"EVICTWINDOW"設成0.5
                    
    /src/syssim_driver.c :
    "HINTGROUP"   : <例> 論文中"HINTGROUP"設成5
    "alpha"       : 預測公式使用的alpha
                    <例> 論文中"alpha"設成0.8

```
>I choose a lazy person to do a hard job. Because a lazy person will find an easy way to do it. [name=Bill Gates]

## 執行結果


**參數顯示**
```cmd
    [YIFEN] HINTGROUP=5, alpha=0.8
    [YIFEN] Trace File=/home/osnet/SNIA_trace/FIU/User2/run1_asim_usr21.txt, max_req=1, MEM Size=8740
    [YIFEN] RWRATIO=0.4, EVICTWINDOW=0.5
    [YIFEN] WB_size = 4000
```
    
**Write Buffer內部資訊**
```cmd
    [YIFEN] write hit count = 6896 write miss count = 56657 write hit rate = 0.108508
    [YIFEN] read hit count = 2969 read miss count = 69766 read hit rate = 0.040819
    [YIFEN] all hit rate = 0.072383
```
**Striping 資訊**
```cmd
    [YIFEN] kick_page_striping_page_count=9662, kick_block_striping_page_count=42995, kick_all_page_count = 52657
```
** GC **
```cmd
    ytc94u total_live_page_cp_count2 = 0,total_gc_count = 0
    ytc94u average block associate logical block = 9.529412
```
**Response time**
```cmd
    all response time: n=34684 average=0.236917 std. deviation=0.176167
    write response time: n=17214 average=0.308586 std. deviation=0.181663
    write_page response time: n=63553 average=0.083584 std. deviation=0.166569
    read response time: n=17470 average=0.166298 std. deviation=0.138018
    read_page response time: n=72735 average=0.039943 std. deviation=0.098093
```

