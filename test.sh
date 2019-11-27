#!/bin/bash
TEST_SIZE=$5

count1=0
count2=0
count3=0
count4=0

echo "$1 got top score" > win1.line
echo "$2 got top score" > win2.line
echo "$3 got top score" > win3.line
echo "$4 got top score" > win4.line

n=($1 $2 $3 $4)



for ((i=0; $i < $TEST_SIZE; i++))
do
	r=$RANDOM
	let "r %= 4"
	./Game ${n[$r]} ${n[((($r+1)%4))]} ${n[((($r+2)%4))]} ${n[((($r+3)%4))]} -s $i -i default.cnf -o default.res 2> out.err
	if grep -c "$1 got top score" out.err
	then
		let "count1+=1"
	else 
		let "count1+=0"
	fi

	if grep -c "$2 got top score" out.err
	then
		let "count2+=1"
	else 
		let "count2+=0"
	fi

	if grep -c "$3 got top score" out.err
	then
		let "count3+=1"
	else 
		let "count3+=0"
	fi

	if grep -c "$4 got top score" out.err
	then
		let "count4+=1"
	else 
		let "count4+=0"
	fi
	rm out.err
	echo "game $i completed"
done
rm win1.line win2.line win3.line win4.line
echo "Resultados:"
echo "  $1: $count1"
echo "  $2: $count2"
echo "  $3: $count3"
echo "  $4: $count4"