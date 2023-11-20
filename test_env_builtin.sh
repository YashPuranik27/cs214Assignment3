echo "Home directory: $HOME"
cd 
pwd
echo "Changing to a temporary directory"
mkdir -p /tmp/mysh_test
cd /tmp mysh_test
pwd
cd -
echo "removing the temporary directory"
rm -r /tmp/mysh_test
echo "Temporary directory removed"