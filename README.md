# mtd_runtime_partition
This is some old code I wrote a few years ago to resize MTD partitions on an embedded system while Linux is running.

It is based off of Linux MTD Utils, I used the following links as a starting point:
* http://article.gmane.org/gmane.linux.drivers.mtd/30949
* http://article.gmane.org/gmane.linux.drivers.mtd/30950
* http://article.gmane.org/gmane.linux.drivers.mtd/30951
* http://www.linux-mtd.infradead.org/


My code assumes the following partition structure:
* mtd0: linux
* mtd1: jffs2
* mtd2: uboot config
* mtd3 does not exist

1. User provides the hex value for number of bytes by which to expand `mtd0`.
1. It works by sending a custom "Shift" message to the driver telling it to move `mtd1` over.
1. The driver actually resizes `mtd0` to the desired size, but instead of shrinking `mtd1` it creates a new `mtd3` of the correct start position and size that `mtd1` should have been (at the time I could not figure out how to shrink `mtd1`).

This allowed me to create a script that burned a new image into `mtd0` and moved my JFFS2 files over to the new partition `mtd3`. On restart, the new image contained an updated Device Tree with the new partition boundaries/sizes.

For more details see: `http://stackoverflow.com/questions/10836715/resize-mtd-partitions-at-runtime`

**Note:** I am no longer maintaining this, just posting it in case anyone is trying to tackle a similar issue.
