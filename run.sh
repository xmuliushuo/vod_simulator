#!/bin/bash

algo=LFU
maxout=1
maxin=1

thresh=0

while [ $thresh -le 600 ]
do
	./st -s -a $algo -o $maxout -i $maxin -l $thresh > serverlog_${algo}_${maxout}_${maxin}_${thresh} &
	sleep 5s
	./st -c > clientlog_${algo}_${maxout}_${maxin}_${thresh} &

	sleep 5500s
	let thresh+=100
done
