struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)要赋值为「系统当前空闲内存的字节数」（不是 “页数”，是字节数）；
  //系统中状态不是 UNUSED 的进程总数」（UNUSED 是 xv6 进程的一种状态，
  //代表 “这个进程槽位未被分配 / 空闲”，所以要统计的是 “正在用的进程数”）。
  uint64 nproc;     // number of process
};
