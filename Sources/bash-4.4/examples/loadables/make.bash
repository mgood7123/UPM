cd /UPM/Sources/bash-4.4/examples/loadables
make mostlyclean
make everything
for i in *.so
    do
        enable -f /UPM/Sources/bash-4.4/examples/loadables/$i ${i/.so}
done
