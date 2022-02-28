#!/bin/bash 
if [ $# -lt 2 ]; then
    echo "Please Input Device ID, e.g.: ./run_zayk_capbenchmark.sh <dev id> <process/thread cnt> <time>"
    exit
fi

status=`lsmod | grep rsp | wc -l`
if [ $status -eq 0 ]; then
    echo "No RSP Driver Found!"
    exit
fi

pnt_cnt=1        
thread_cnt=$2

test_time=2s
if [[ $3 -ne 0 ]]; then
    test_time=$3s
fi

trap 'onCtrlC' INT
function onCtrlC () {
    echo -e '\n Ctrl+C is captured, Test Stop !\n'
    killall -w -g run_zayk_capbenchmark.sh 2> /dev/null
    exit
}

# size_array=(8192)
size_array=(64 128 256 512 1024 1440 2048 4096)

function zayk_capbenchmark_test()
{
    devid=$1

    ct=$(date "+%Y-%m-%d %H:%M:%S")
    echo -e -n "\n$ct: DEV-$devid:$(eval echo '$'bdf) / Total Devices: $devnum;\n"

    echo -e -n "\nSync Mode [ "

    for (( index = 0; index < ${#size_array[*]}; index++ ));
    do
        size=${size_array[$index]}
        echo -n "$size "
    done
    echo -e "] Performance Test Start.\n"

    echo -e "\nConfig: p-t:[$pnt_cnt*$thread_cnt]\n"


#SM2
    sm2index_array=(29 30 31 32 33)
    for (( sm2_index = 0; sm2_index < ${#sm2index_array[*]}; sm2_index++ ));
    do

        i=${sm2index_array[$sm2_index]}
        ./zayk_capbenchmark.out $i 2 1 $thread_cnt 32 $test_time 0 $devid 0 0 | grep -i mbps | awk '{printf "%-32s: %-16s %-16s\n",$1, $4, $5}'
    
    done

#RSA
    rsaindex_array=(34 38 40 41 45 47)
    for (( rsa_index = 0; rsa_index < ${#rsaindex_array[*]}; rsa_index++ ));
    do

        i=${rsaindex_array[$rsa_index]}
        ./zayk_capbenchmark.out $i 2 1 $thread_cnt 256 $test_time 0 $devid 0 0 | grep -i mbps | awk '{printf "%-32s: %-16s %-16s\n",$1, $4, $5}'
    
    done

    for (( index = 0; index < ${#size_array[*]}; index++ ));
    do
        size=${size_array[$index]}

        echo -e "\nSize = $size\n"

# TRNG 随机数
        if [[ $size -lt 8192 ]]; then
            ./zayk_capbenchmark.out 54 2 1 $thread_cnt $size $test_time 0 $devid 0 0 | grep -i mbps | awk '{printf "%-32s: %-16s\n",$1,$6}'

        fi
#HASH [ 0 1 2 4]
        hashindex_array=(0 1 2 4)
        for (( hash_index = 0; hash_index < ${#hashindex_array[*]}; hash_index++ ));
        do

            i=${hashindex_array[$hash_index]}
            ./zayk_capbenchmark.out $i 2 1 $thread_cnt $size $test_time 0 $devid 0 0 | grep -i mbps | awk '{printf "%-32s: %-16s %-16s\n",$1, $4, $6}'
        
        done

#对称算法 16-26 [8,9,12,13,16,17,19,20,23,24,25,26]
        symmindex_array=(16 17 19 20 8 9 12 13 23 24 25 26) #sm1 sm4 aes des
        
        for (( symm_index = 0; symm_index < ${#symmindex_array[*]}; symm_index++ ));
        do

            i=${symmindex_array[$symm_index]}
            ./zayk_capbenchmark.out $i 2 1 $thread_cnt $size $test_time 0 $devid 0 0 | grep -i mbps | awk '{printf "%-32s: %-16s %-16s\n",$1, $4, $6}'
        
        done
    done

    ct=$(date "+%Y-%m-%d %H:%M:%S")
    echo -e -n "\n$ct: Dev-$1, Bus:$(eval echo '$'bdf), $mode mode [ "
    for (( index = 0; index < ${#size_array[*]}; index++ ));
    do
        size=${size_array[$index]}
        echo -n "$size "
    done
    echo -e "] Performance Test Stop.\n"
}

if [ $1 -eq 0 ]; then

# test all
    for ((dev_index=1; dev_index<=$devnum; dev_index++))
    do
        bdf=`./tools 3 1 | awk 'NR==2{print $4}' | awk -F'[-,]' '{print $2}'`
        if [ $bdf == ":" ]; then
            echo "ERROR: The device number is invalid"
            exit;
        fi

        if [[ $sync_mode -eq 1 ]]; then
            zayk_capbenchmark_test $dev_index 0
        else
            zayk_capbenchmark_test $dev_index 2
        fi

    done
else
    zayk_capbenchmark_test $1 #同步
fi