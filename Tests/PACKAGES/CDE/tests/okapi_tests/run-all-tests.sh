for t in `ls test-*.sh`
do
  echo $t
  ./$t
done
