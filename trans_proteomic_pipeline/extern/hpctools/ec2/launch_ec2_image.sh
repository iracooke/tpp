#!/bin/bash 
#
# Script for publishing a TPP Amazon EC2 image. 
# INCOMPLETE
#

# Use sudo to run commands as root
EC2_AMI=ami-08f40561

echo "EC2_AMI = ${EC2_AMI}"

echo "=== Launching image..."
OUT=`ec2-run-instances $EC2_AMI -k ISB -n 1 -g TPP -t m2.xlarge`
echo $OUT
ID=`echo $OUT | grep RESERVATION | cut -f6`
STATUS=`ec2-describe-instances $ID | grep INSTANCE | cut -f4,6`
echo $STATUS
EC2=`echo $STATUS | cut -f1`
echo "DNS $EC2"
echo "=== Done launching image"
echo

#echo "=== Copying files..."
#scp -i ~/.ec2/ISB.pem $EC2_PRIVATE_KEY  ubuntu@$EC2:/tmp
#echo "=== Done copying files"
#echo
