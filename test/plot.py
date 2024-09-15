import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime, timedelta
import numpy as np
import os


def read_enu_data(file_path):
    e_data = []
    n_data = []
    u_data = []

    i = 0
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for line in lines:
            if line.strip() and line[4].isdigit():  # 只处理非空行
                columns = line.split()
                if len(columns) == 3:  # 确保行中有三列数据
                    try:
                        e_data.append(float(columns[0]))
                        n_data.append(float(columns[1]))
                        u_data.append(float(columns[2]))
                    except ValueError:
                        # 忽略无法转换为浮点数的行
                        continue

    return e_data, n_data, u_data


def plot_enu(time_series, e_data, n_data, u_data, title="ENU ERROR PLOT", savepath=None):
    # 创建 3 行 1 列的子图
    fig, axs = plt.subplots(3, 1, figsize=(10, 8), sharex=True)
    edata = []
    ndata = []
    udata = []

    for e in e_data:
        edata.append(e*e)
    for n in n_data:
        ndata.append(n*n)
    for u in u_data:
        udata.append(u*u)

    e_rms = np.sqrt(np.mean(edata))
    n_rms = np.sqrt(np.mean(ndata))
    u_rms = np.sqrt(np.mean(udata))

    print(e_rms, n_rms, u_rms)
    # 绘制E方向误差
    axs[0].plot(time_series, e_data, 'b', label = f'RMS = {e_rms:.4f}')
    axs[0].set_ylabel('E Error (m)')
    axs[0].set_title(title)
    axs[0].grid(True)
    axs[0].legend(loc = 'upper right')

    # 绘制N方向误差
    axs[1].plot(time_series, n_data, 'g', label = f'RMS = {n_rms:.4f}')
    axs[1].set_ylabel('N Error (m)')
    axs[1].grid(True)
    axs[1].legend(loc = 'upper right')

    # 绘制U方向误差
    axs[2].plot(time_series, u_data, 'r', label = f'RMS = {u_rms:.4f}')
    axs[2].set_ylabel('U Error (m)')
    axs[2].set_xlabel('Time (UTC)')
    axs[2].grid(True)
    axs[2].legend(loc = 'upper right')

    # 设置横坐标格式为时间格式，并旋转时间标签
    axs[2].xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
    axs[2].xaxis.set_major_locator(mdates.HourLocator(interval=1))
    fig.autofmt_xdate()

    plt.tight_layout()

    if savepath:
        plt.savefig(savepath)
        print(f"Figure saved to {savepath}")

    # plt.show()

filepath = '/mnt/share/share/SPP/res_bjfs'

os.chdir(filepath)
j = 0
for data in np.sort(os.listdir()):
    fname = data.split('.')
    if fname[1] == "png":
        continue
    pname = fname[0] + '.' + "png"
    spath = '/'.join([filepath,pname])
    ymd = [2024, 4, 9]
    e_data, n_data, u_data = read_enu_data(file_path=data)
    start_time = datetime(int(ymd[0]), int(
                ymd[1]), int(ymd[2]), 0, 0, 0)
    time_interval = timedelta(seconds=30)
    time_series = [start_time + i * time_interval for i in range(2880)]
    plot_enu(time_series, e_data, n_data, u_data, title=f"ENU Errors for {fname[0]}", savepath=spath)
    print(f"process complete!, outfile path = {spath}")
    j += 1