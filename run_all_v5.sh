#!/bin/bash
#######################################################################################################################################

###################################	
#this variable  == 0 represent the buffer and ftl are work cooperation, it == 1 is traditional lru policy
buffer_no_parallel=0
striping_threshold_method=3
date=$(date "+%Y_%m_%d_%H")
cache=16000
clear_statistic_data_req_count=0


function execite_a_locality_workload()
{
	#$1 == $para ; $2 == $input ; $3 == $cache ; $4 == $output  ;	$5 == date ; $6 == striping threshold ; $7 == clear stasitcs data req count
	#$8 == $striping_threshold_method

	outputfile="$5_$4_$6_$8_$buffer_no_parallel"
	echo "initial with locality allocation">>$outputfile

	echo "$1 $2 $3 $4 $6 $8">>$outputfile
	
	./src/disksim $1 stdout ascii $2  0  $3 99999 2 $6 $7 $buffer_no_parallel $8 |egrep  -n 'ytc94u' >> $outputfile
	
}
#######################################################################################################################################

function execite_a_dynamic_workload_init_locality()
{
	#$1 == $para ; $2 == $input ; $3 == $cache ; $4 == $output

	outputfile="$5_$4_$6_$8_$buffer_no_parallel"
	echo "dynamic and initial with locality allocation">>$outputfile
	echo "$1 $2 $3 $4 $6 $8">>$outputfile
	
  ./src/disksim $1 stdout ascii $2  0  $3 999999 2 $6 $7 $buffer_no_parallel $8  |egrep  -n 'ytc94u' >> $outputfile
	#999999 represent the striping mechanism will been open with read dominate trace and write dominate trace will use locality method

}
#######################################################################################################################################

function execite_a_dynamic_workload_init_striping()
{
	#$1 == $para ; $2 == $input ; $3 == $cache ; $4 == $output

	outputfile="$5_$4_$6_$8_$buffer_no_parallel"
	echo "dynamic and initial with striping allocation">>$outputfile
	echo "$1 $2 $3 $4 $6 $8">>$outputfile
	
	./src/disksim $1 stdout ascii $2  0  $3 999999 1 $6 $7 $buffer_no_parallel $8 |egrep  -n 'ytc94u' >> $outputfile
	#999999 represent the striping mechanism will been open with read dominate trace and write dominate trace will use locality method

}
#######################################################################################################################################
#execite_a_striping_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
function execite_a_striping_workload()
{
	#$1 == $para ; $2 == $input ; $3 == $cache ; $4 == $output

	outputfile="$5_$4_$6_$8_$buffer_no_parallel"
	echo "initial with striping allocation">>$outputfile
	echo "$1 $2 $3 $4 $6 $8">>$outputfile
	
  ./src/disksim $1 stdout ascii $2  0  $3 0 1 $6 $7 $buffer_no_parallel $8 |egrep  -n 'ytc94u' >> $outputfile
  #./src/disksim ./ssdmodel/valid/ssd-iozone.parv stdout ascii ./src/iozone 0 16000 0 1 11_3 0 0 3
#	exit 0
}
#######################################################################################################################################
function run_all()
{
#---------------------VMM V11-------------------------------------------------WR

	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/vmm_v11.dis_reserve_rate_8
	output=16000_vmm_v11
#	clear_statistic_data_req_count=3309622
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#------------------Disp Data----------------------------------------------------------R
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/DispAdsData_second_80G.disX3
	output=16000_DispAdsDataX3
#	clear_statistic_data_req_count=2719327
#	execite_a_striping_workload $para $input $cache $output	$date	$1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#------------------DEVELOP TOOL REVERSE----------------------------------------------------------W
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/Develop_tool_reverse_first_80G_disk6.dis_reserve_rate_8
	output=16000_develop_tool_reverse_first_80G
#	clear_statistic_data_req_count=4934947
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#-------------------MSN_FILESERVER----------------------------------------------------------W
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/MSN_fileserver.dis_reserve_rate_8X4
	output=16000_MSN_FILESERVERX4
#	clear_statistic_data_req_count=4025052
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
	
#exit 0
#-------------------2win+2websearch ----------------------------------------------------------
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/winX1+websearch_traceXn.dis_cut_shot_down
	output=16000_conbine_winX1+websearchXn
#	clear_statistic_data_req_count=16883038
#	execite_a_striping_workload $para $input $cache $output	$date	$1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#-------------------WIN PC_v1(FAT)----------------------------------------------W
		para=./ssdmodel/valid/ssd-win-5-1_v2_bg.parv
		input=../trace/windowsX2.dis_reserve_rate_6
		output=16000_win_V1X2_dynamic
#		clear_statistic_data_req_count=4190468
#		execite_a_striping_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#		execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#-----------------------WINPC_v2(NTFS)------------------------------------------------------
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/winPC_v2_disk0.log_reserve_rate_8
	output=16000_winpc_v2
#	clear_statistic_data_req_count=7359186
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#-------------------WEB SEARCH-----------------------------------------------
	#para=./ssdmodel/valid/ssd-win-5-1_v2_bg.parv
	para=./ssdmodel/valid/ssd-iozone.parv
	#input=../trace/websearch3_500M.dis_reserve_rate_8
	input=./src/iozone
	output=16000_websearch3
#	clear_statistic_data_req_count=2841136
	execite_a_striping_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
exit 0
#-------------------MP3 READ-------------------------------------------------------
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/mp3_read_v4.dis_reserve_rate_8
	output=16000_mp3_read
#	clear_statistic_data_req_count=2257102
#	execite_a_striping_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#---------------------SQL V3---------------------------------------------
		para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
		input=../trace/sqlsim_sort_v3.dis_reserve_rate_8
		output=16000_sqlsim_v3
#		clear_statistic_data_req_count=3193204
#		execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#		execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#exit 0
#----------------------synthesize--------------------------------------------
#																																										note : the trace will run so long
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/synthesize_WRWR_v17.dis
	output=16000_synthesize_WRWR_v17_dynamic_locality
	clear_statistic_data_req_count=0
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method

#	exit 0

#----------------------sql and mp3 read--------------------------------------------
#																																										
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/sqlsim_sort_v3_and_mp3_read_v4.dis
	output=slqsim_and_mp3_read
	clear_statistic_data_req_count=0
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method

#	exit 0

#----------------------win v1 and web search--------------------------------------------
#																																										
	para=./ssdmodel/valid/ssd-win-5-1_v2_bg.parv
	input=../trace/FAT_WEB.dis
	output=FAT_WEB3
	clear_statistic_data_req_count=0
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method

#	exit 0



#----------------------dev and mp3 disp--------------------------------------------
#																																										
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/dev_and_disp.dis
	output=dev_and_disp
	clear_statistic_data_req_count=0
#	execite_a_striping_workload $para $input $cache $output	$date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_locality_workload $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $1 $clear_statistic_data_req_count $striping_threshold_method

#	exit 0

}
#######################################################################################################################################
read -p "Do you want to debug  (y/n): " yn2

if [ "$y2n" == "N" ] || [ "$yn2" == "n" ]; then
	
#	striping_threshold="$((5*8))_8"
#	striping_threshold=11

#	make clean
#	make

#	striping_threshold="$((18))_0.5"

	striping_threshold="$((11))_3"
	run_all $striping_threshold
	exit 0
	striping_threshold="$((14))"
	run_all $striping_threshold
	striping_threshold="$((15))"
	run_all $striping_threshold

	exit 0
	striping_threshold="$((15))"
	run_all $striping_threshold

	exit 0
	striping_threshold="$((32))"
	run_all $striping_threshold

#	striping_threshold="$((19))_0.5"

#	striping_threshold="$((32))"
#	run_all $striping_threshold

fi

echo "success"
exit 0

#------------------------------------------------------------------------			
#	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#	input=../trace/build_server.dis
#	output=20000_build_server
#	execite_a_striping_workload $para $input $cache $output	$date	
#	execite_a_locality_workload $para $input $cache $output $date
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#exit 0	

#--------------------------------------------------------------------------
#	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#	input=../trace/winPC_v3_disk0.log
#	output=20000_winPC_v3_disk0
#	execite_a_striping_workload $para $input $cache $output	$date
#	execite_a_locality_workload $para $input $cache $output $date
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date

#-------------------WIN PC----------------------------------------------
#		output=20000_winX2_threshold_10_miss_record_5X63
#	 	input=../trace/windowsX2.dis
#		para=./ssdmodel/valid/ssd-win-2-1_v2_bg.parv
#		execite_a_striping_workload $para $input $cache $output $date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date
#exit 0

#---------------------POSTMARK--------------------------------------------
#		input=../trace/postmark5_align.dis_reserve_rate_7
#		output=20000_postmark5
#		execite_a_striping_workload $para $input $cache $output $date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date

#----------------------------------------------------------------
#		input=../trace/msn_fileserver_disk0.dis_reserve_rate_7
#		output=20000_msn_fileserver_disk0
#		execite_a_striping_workload $para $input $cache $output	
#		execite_a_locality_workload $para $input $cache $output
#		execite_a_dynamic_workload $para $input $cache $output


#---------------------SQL---------------------------------------------
#		para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#		input=../trace/sqlsim2.dis_reserve_rate_10
#		output=20000_sqlsim2
#		execite_a_striping_workload $para $input $cache $output	$date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date
#---------------------SQLX2---------------------------------------------
#		para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#		input=../trace/sqlsim2.dis_reserve_rate_10X2
#		output=20000_sqlsim2X2
#		execite_a_striping_workload $para $input $cache $output	$date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date
#---------------------SQLX5---------------------------------------------
#		para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#		input=../trace/sqlsim2.dis_reserve_rate_10X5
#		output=20000_sqlsim2X5
#		execite_a_striping_workload $para $input $cache $output	$date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date

#---------------------VMM V4-------------------------------------------------
#		input=../trace/vmm_v6.dis_reserve_rate_8
#		output=20000_vmm_v6
#		execite_a_striping_workload $para $input $cache $output	$date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date

#---------------------VMM V4X2-------------------------------------------------
#		input=../trace/vmm_v6.dis_reserve_rate_8X2
#		output=20000_vmm_v6X2
#		execite_a_striping_workload $para $input $cache $output	$date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date

#---------------------VMM V4X5-------------------------------------------------
#		input=../trace/vmm_v6.dis_reserve_rate_8X5
#		output=20000_vmm_v6X5
#		execite_a_striping_workload $para $input $cache $output	$date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date


#---------------------VMM V4X5-------------------------------------------------
#		input=../trace/vmm_v6.dis_reserve_rate_8X10
#		output=20000_vmm_v6X10
#		execite_a_striping_workload $para $input $cache $output	$date
#		execite_a_locality_workload $para $input $cache $output $date
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date


#-------------------MRS----------------------------------------------------------
#	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#	input=../trace/mrs.dis_reserve_rate_8
#	output=20000_mrs
#	execite_a_striping_workload $para $input $cache $output	$date	
#	execite_a_locality_workload $para $input $cache $output $date
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#exit 0

#-------------------live map----------------------------------------------------------
#	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#	input=../trace/LIVE_disk1.dis_reserve_rate_8X5
#	output=20000_live_disk1
#	execite_a_striping_workload $para $input $cache $output	$date	
#	execite_a_locality_workload $para $input $cache $output $date
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date
#exit 0

#----------------------IOZONE RWX10--------------------------------------------------
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/iozone_rw_v5.dis_reserve_rate_8X10
	output=20000_iozone_rw_v5X10
	claear_statistic_data_req_count=
#	execite_a_striping_workload $para $input $cache $output	$date $striping_threshold
#	execite_a_locality_workload $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $striping_threshold

#-------------------build serverX20 ----------------------------------------------------------R
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/build_server_eighth_80G.dis_reserve_rate_8X20
	output=20000_build_server_eighth80X20
	claear_statistic_data_req_count=
#	execite_a_striping_workload $para $input $cache $output	$date	$striping_threshold
#	execite_a_locality_workload $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $striping_threshold
#exit 0	


#------------------Disp DataX2----------------------------------------------------------R
#	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
#	input=../trace/DispAdsData_second_80G.dis_reserve_rate_8X2
#	output=16000_DispAdsDataX2
#	claear_statistic_data_req_count=
#	execite_a_striping_workload $para $input $cache $output	$date	$striping_threshold
#	execite_a_locality_workload $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $striping_threshold


#-----------------------WINPC_v2X2(NTFS)----------------------------------------------------	
	para=./ssdmodel/valid/ssd-sql-5-1_v2_bg.parv
	input=../trace/winPC_v2_disk0.log_reserve_rate_8X2
	output=16000_winpc_v2X2
	claear_statistic_data_req_count=
#	execite_a_striping_workload $para $input $cache $output	$date $striping_threshold
#	execite_a_locality_workload $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_striping $para $input $cache $output $date $striping_threshold
#	execite_a_dynamic_workload_init_locality $para $input $cache $output $date $striping_threshold

#-------------------WIN PC_v1X2(FAT)----------------------------------------------W
		para=./ssdmodel/valid/ssd-win-2-1_v2_bg.parv
		input=../trace/windowsX2.dis
		output=16000_win_V1X2
		claear_statistic_data_req_count=
#		execite_a_striping_workload $para $input $cache $output $date $striping_threshold
#		execite_a_locality_workload $para $input $cache $output $date $striping_threshold
#		execite_a_dynamic_workload_init_striping $para $input $cache $output $date $striping_threshold
#		execite_a_dynamic_workload_init_locality $para $input $cache $output $date $striping_threshold


