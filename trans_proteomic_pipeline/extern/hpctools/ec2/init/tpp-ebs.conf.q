# tpp-ebs - task for detecting attached/detached EBS blocks
#
# This is a Ubuntu upstart script for managing elastic block stores on Amazon's
# EC2 cloud service.  It detects events emitted from the upstarte-udev-bridge
# to handle attachments/detachments of EBS stores on the node.
#
# For more information on how to install and use this script please see Ubuntu
# upstart (http://upstart.ubuntu.com/).
#
description     "Mounts/unmounts EBS storage in /mnt/tppdata"
author          "Joe Slagel <jslagel@systemsbiology.org>"

start on block-device-added or block-device-removed

env MOUNT=/mnt/tppdata

task
console log

script

    echo "tpp_ebs: detected ${ACTION} of $DEVNAME" | wall
    if [ "${ACTION}" = "add" ]; then
        if [ "${ID_FS_TYPE}" = "" ]; then
	   ID_FS_TYPE=ext3
           echo "tpp_ebs: device isn't formatted so I will as $ID_FS_TYPE" | wall
	   mkfs -t $ID_FS_TYPE $DEVNAME
        fi

	# Find an empty mount point
        for N in 1 2 3 4 5 6 7 8 9; do 
           EBS="ebs$N"; 
           [ -e "$MOUNT/$EBS" ] || break; 
        done
        EBS=${EBS=ebs1}

	# Mount it
        echo "tpp_ebs: mounting $DEVNAME to $MOUNT/$EBS" | wall
        mkdir -p $MOUNT/$EBS
        mount -t $ID_FS_TYPE $DEVNAME $MOUNT/$EBS
        chown www-data:www-data $MOUNT/$EBS

	# So if we are rebooted it gets remounted
        echo "$DEVNAME $MOUNT/$EBS ext3 defaults 0 0" >> /etc/fstab
    fi

    if [ "$ACTION" = "remove" ] && [ "$DEVTYPE" = "disk"]; then
        echo "tpp_ebs: unmount $DEVNAME" | wall
        umount $DEVNAME
	perl -ni -e "print unless m{$DEVNAME}" /etc/fstab
        # don't worry about removing the mount directory users won't
        # be able to write files to it
    fi

    echo "tpp_ebs: done" | wall
end script

