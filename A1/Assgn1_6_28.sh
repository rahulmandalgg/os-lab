#!/bin/bash
> output.txt
file=$1
for ((i=2;i<=1000;i++));do
        arr[$i]=1
    done

for ((i=2;i<=1000;i++));do  	
    	if [[ ${arr[$i]} -ne 0 ]]; then
    		  for (( j=$((i*2)); j<=1000; j+=$i )); do
    		  	 arr[j]=0
    		  done
    	fi
done
a=2
b=0
for ((i=2;i<=1000;i++));do
	if [[ ${arr[$i]} -ne 0 ]]; then
		#echo $a
		arr1[b]=$((a))
		b=$((b+1))
	fi
	a=$((a+1))
    done

while IFS= read -r line
do
    num="$((${line//[ $'\001'-$'\037']}))"
    for prime in ${arr1[@]}; do
    	if [[ $((num%prime)) -eq 0 ]]; then
    		echo -n $prime" " >> output.txt
    	fi
    done 
    echo "" >> output.txt
    echo "" >> output.txt
done <"$file"
