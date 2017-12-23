#!/bin/ash
if [ $# -lt 1 ]; then

	echo "install finish."
	exit 0
fi

echo "start packet install......."
echo "modify the musicpwd to 123456"
uci -c/opt/ft set ftconfig.@ftconfig[0].musicpwd=123456
uci commit

src_path=`echo $1|awk '{sub(/^[[:blank:]]*/,"",$0); print $0;}'`
#echo "src_path="$src_path

## cover old file and add new file
array_cover0="/opt/ft/ftconfig"

array_src0=$src_path/ftconfig


cover_len=0
src_len=0
del_len=0

i=0
while [ $i -lt $src_len ]; do
	#eval echo \$array_src$i
	eval tmp="\$array_src$i"
	if [ ! -s $tmp ]; then
		echo "Install error :"$tmp" is not exist ..."
		exit 1
	fi
	let i++
done

killall -9 smarthome.out
sleep 1
## delete old file
i=0
while [ $i -lt $del_len ]; do
	eval tmp_del="\$array_del$i"
	echo "delete "$tmp_del"..."
	rm -rf  $tmp_del -f
	echo "delete old file ok "
	let i++
done



sleep 3
i=0
while [ $i -lt $src_len ]; do
	eval tmp_src="\$array_src$i"
	eval tmp_dst="\$array_cover$i"
	echo "Install "$tmp_dst"..."
	cp $tmp_src $tmp_dst -f
	chmod +x $tmp_dst
	echo " ok "
	let i++
done
	
echo "install finish."

exit 0
