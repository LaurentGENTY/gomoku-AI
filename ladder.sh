#/bin/bash

N=1
MINSIZE=10
MAXSIZE=10
PLAYERS=(lambda.so player4.so player4.2.so player4.4.so player1.so defense_player.so player4.3.so delta_player2.so)
NB_PLAYERS=7
declare -A score
score["Lambda"]=0
score["4.2"]=0
score["4.1"]=0
score["4.4"]=0
score["4.3"]=0
score["v2"]=0
score["Defense"]=0
score["stupid"]=0
declare -A match_nul
match_nul["Lambda"]=0
match_nul["4.2"]=0
match_nul["4.1"]=0
match_nul["4.4"]=0
match_nul["4.3"]=0
match_nul["v2"]=0
match_nul["Defense"]=0
match_nul["stupid"]=0
declare -A ladder
ladder["Lambda"]=0
ladder["4.2"]=0
ladder["4.1"]=0
ladder["4.4"]=0
ladder["4.3"]=0
ladder["v2"]=0
ladder["Defense"]=0
ladder["stupid"]=0
nb_match=0

victory()
{
	for str in ${!score[@]} ; do
		if [ -n "`echo "$1" | grep ${str}`" ] ; then
			score[$str]=$((score[$str]+1))
			ladder[$str]=$((ladder[$str]+2))
		fi
	done
}

equality()
{
	for str in ${!score[@]} ; do
		if [ -n "`echo "$1" | grep ${str}`" ] ; then
			match_nul[$str]=$((match_nul[$str]+1))
			ladder[$str]=$((ladder[$str]+1))
		fi
	done
}

for i in `seq 0 $NB_PLAYERS` ; do
	for j in `seq 0 $NB_PLAYERS` ; do
		if [ $i != $j ] ; then
			for SIZE in `seq ${MINSIZE} ${MAXSIZE}` ; do
				nb_match=$((nb_match+1))
				echo -n "size = ${SIZE} ; first : ${PLAYERS[i]} - second : ${PLAYERS[j]} => "
				RES=`./install/server -n ${SIZE} install/${PLAYERS[j]} install/${PLAYERS[i]} | tail -n1`
				echo "$RES"
				if [ -n "`echo "$RES" | grep Egalité`" ] ; then
					equality "$RES"
				else
					victory "$RES"
				fi
			done
		fi
	done
	echo "====================================================================="
done

for str in ${!score[@]} ; do
	echo "${str} a gagné ${score[${str}]} fois sur $nb_match et fait ${match_nul[${str}]} matchs nuls (total ${ladder[${str}]})"
done
