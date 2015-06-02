for i in `seq 1 5`; do
  socat STDIO TCP6:localhost:12346 >buffer$i.out &
done
