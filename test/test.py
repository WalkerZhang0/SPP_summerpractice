from glob import glob
from os.path import join
import subprocess
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime, timedelta
import numpy as np

# ofile = "/mnt/share/share/SPP/data/2022-4-9/slac0991.22o"
# nfile = "/mnt/share/share/SPP/data/2022-4-9/BRDM00DLR_S_20220990000_01D_MN.rnx"
# runpath = "/mnt/share/share/SPP/build/SPP"
# outpath = "/mnt/share/share/SPP/data/2022-4-9/res.txt"

start_time = datetime(2022,4,9,0,0,0)
time_interval = timedelta(seconds = 30)
time_series = [start_time + i*time_interval for i in range(2880)]

e_data = np.random.rand(len(time_series)) * 0.1  # E方向误差
n_data = np.random.rand(len(time_series)) * 0.2  # N方向误差
u_data = np.random.rand(len(time_series)) * 0.3  # U方向误差

fig, axs = plt.subplots(3, 1, figsize=(10, 8), sharex=True)

# 绘制
# 绘制E方向误差
axs[0].plot(time_series, e_data, 'b')
axs[0].set_ylabel('E Error (m)')
axs[0].set_title('ENU Error Plot')
axs[0].grid(True)

# 绘制N方向误差
axs[1].plot(time_series, n_data, 'g')
axs[1].set_ylabel('N Error (m)')
axs[1].grid(True)

# 绘制U方向误差
axs[2].plot(time_series, u_data, 'r')
axs[2].set_ylabel('U Error (m)')
axs[2].set_xlabel('Time (UTC)')
axs[2].grid(True)

# 设置横坐标格式为时间格式，并旋转时间标签
axs[2].xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
axs[2].xaxis.set_major_locator(mdates.HourLocator(interval=1))
fig.autofmt_xdate()

plt.tight_layout()
plt.show()