In the root directory run
make
sudo insmod gu_page.ko

to confirm it has loaded
tail /var/log/syslog

or /var/log/messages depending on system
Should see "loaded the gu_page"

You need to create a device file to talk
to the char device. In the userspace dir
mknod simpleuser c 42
make

Then run as root
./userspace simpleuser

Check the kernel messages for page addr
tail /var/log/syslog

To remove the lkm
sudo rmmod gu_page

Check removal
tail /var/log/syslog
