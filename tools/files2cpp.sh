#/bin/sh
output=$1
suffix=$2
shift; shift
echo '// generated file: DO NOT EDIT!' > output
for f in $@ ; do
    echo "// input file: $f" >> output
    varname=`basename $f $suffix`
    echo "std::string $varname =" >> $output
    hexdump -v -e '16/1 "_x%02X" "\n"' $f | sed -e 's/_x  //g' -e 's/_x/\\x/g' -e 's/.*/    "&"/' >> $output
    echo ";" >> $output
done
echo "end of generated file" >> output
