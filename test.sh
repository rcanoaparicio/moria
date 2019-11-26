#!/bin/bash
TEST_SIZE=100
player_name="EPSEVG"


count=0
echo "$player_name got top score" > win.line
for ((i=0; $i < $TEST_SIZE; i++))
do
	./Game Dummy Dummy Dummy EPSEVG -s $i -i default.cnf -o default.res 2> out.err
	if grep -Fxq out.err win.line
	then
		let "count+=0"
	else 
		let "count+=1"
	fi
	rm out.err
	echo "game $i completed"
done
rm win.line
echo "Partidas ganadas: $count de $TEST_SIZE"