(
  rulename = "Critical devices",
  severity = $(SIG_HI),
  recurse = false
)
{
     /dev/kmem                         -> $(Device) ;
     /dev/mem                          -> $(Device) ;
     /dev/null                         -> $(Device) ;
     /dev/zero                         -> $(Device) ;
     /proc/devices                     -> $(Device) ;
     /proc/net                         -> $(Device) ;
     /proc/sys                         -> $(Device) ;
     /proc/cpuinfo                     -> $(Device) ;
     /proc/modules                     -> $(Device) ;
     /proc/mounts                      -> $(Device) ;
     /proc/dma                         -> $(Device) ;
     /proc/filesystems                 -> $(Device) ;
     /proc/pci                         -> $(Device) ;
     /proc/interrupts                  -> $(Device) ;
     /proc/driver/rtc                  -> $(Device) ;
     /proc/ioports                     -> $(Device) ;
     /proc/scsi                        -> $(Device) ;
     /proc/kcore                       -> $(Device) ;
     /proc/self                        -> $(Device) ;
     /proc/kmsg                        -> $(Device) ;
     /proc/stat                        -> $(Device) ;
     /proc/ksyms                       -> $(Device) ;
     /proc/loadavg                     -> $(Device) ;
     /proc/uptime                      -> $(Device) ;
     /proc/locks                       -> $(Device) ;
     /proc/version                     -> $(Device) ;
     /proc/mdstat                      -> $(Device) ;
     /proc/meminfo                     -> $(Device) ;
     /proc/cmdline                     -> $(Device) ;
     /proc/misc                        -> $(Device) ;
     /sys/block                        -> $(Device) ;
     /sys/bus                          -> $(Device) ;
     /sys/class                        -> $(Device) ;
     /sys/devices                      -> $(Device) ;
     /sys/firmware                     -> $(Device) ;
     /sys/fs                           -> $(Device) ;
     /sys/kernel                       -> $(Device) ;
     /sys/module                       -> $(Device) ;
     /sys/power                        -> $(Device) ;
}

