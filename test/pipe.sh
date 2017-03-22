| output 

output | 

cat < 

| >

#with connectors
echo hello || cat < newfile | tr a-z A-Z

cat < newfile | tr a-z A-Z && echo hello

#needs ctrl+c to stop
cat >> newfile | tr a-z A-Z

ls < main.cpp | tr a-z A-Z 

cat < output | tr A-Z a-z

#2 pipes
 cat < output | tr A-Z a-z | tee newfile

#3 pipes
 cat < output | tr A-Z a-z | tee newOutputFile1 | tr a-z
A-Z > newOutputFile2


#More than 3
cat < output | tr A-Z a-z | tee newOutputFile1 | tr a-z
A-Z | tee newoutput2 | tr a-z A-Z > newnewFile
#make sure to not confuse with connector
echo hello || ls main.cpp 
