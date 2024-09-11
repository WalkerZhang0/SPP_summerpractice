""" 
get_base_data.py - retrieve base observation and navigation data for the
    2023 GSDC competition 
"""

import os
from os.path import join
from datetime import datetime
import numpy as np
import requests
import gzip
from glob import glob
import subprocess
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime, timedelta

# 判断是否为数字


def is_number(s):
    try:    # 如果能运⾏ float(s) 语句，返回 True（字符串 s 是浮点数）
        float(s)
        return True
    except ValueError:  # ValueError 为 Python 的⼀种标准异常，表⽰"传⼊⽆效的参数"
        pass  # 如果引发了 ValueError 这种异常，不做任何事情（pass：不做任何事情，⼀般⽤做占位语句）
    try:
        import unicodedata  # 处理 ASCII 码的包
        unicodedata.numeric(s)  # 把⼀个表⽰数字的字符串转换为浮点数返回的函数
        return True
    except (TypeError, ValueError):
        pass
        return False


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

    # 绘制E方向误差
    axs[0].plot(time_series, e_data, 'b')
    axs[0].set_ylabel('E Error (m)')
    axs[0].set_title(title)
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

    if savepath:
        plt.savefig(savepath)
        print(f"Figure saved to {savepath}")

    plt.show()


proxies = {
    "http": "http://127.0.0.1:7890",
    "https": "http://127.0.0.1:7890"
}

# Input parameters
datadir = '/mnt/share/share/SPP/data'  # relative to python script
# List of CORS stations to use
# stas = ['slac']  # Bay Area, LA, backup for Bay Area
# stas = ['bjfs', 'shao', 'chan','kunm', 'lhaz', 'urum', 'abpo', 'ade3', 'bamf',
# 'bill', 'chum','dubo']  # Bay Area, LA, backup for Bay Area
stas = ['slac', 'camr', 'kntn']

# site to retrieve base observation data
obs_url_base = 'https://geodesy.noaa.gov/corsdata/rinex'

# site to retrieve satellite navigation data
nav_url_base = 'https://cddis.nasa.gov/archive/gnss/data/daily'  # /2021/342/21p/
nav_file_base = 'brdc'  # 20213420000_01D_MN.rnx.gz
# Access to CDDIS navigation data requires registering for a free account and
# setup of a .netrc file as described at
# https://cddis.nasa.gov/Data_and_Derived_Products/CreateNetrcFile.html.
# Make sure this file  is in the users home directory

# Make sure you have downloaded this executable before running this code
crx2rnx_bin = '/home/walker/RTKLIB/CRX2RNX'  # relative to data directory
convbin = '/home/walker/RTKLIB/app/consapp/convbin/gcc/convbin'
spp = '/mnt/share/share/SPP/build/SPP'

# if not os.listdir(datadir):
#     print(f"No datasets founr ini {datadir}. Please add datasets and try again.")
#     exit()

# Loop through data sets in the data directory
os.chdir(datadir)  # cut work path to datadir
for dataset in np.sort(os.listdir()):    # list all the file name in datadir
    if not os.path.isdir(join(dataset)):  # check dataset is a dir or not
        continue
    print(dataset)
    ymd = dataset.split('-')
    doy = datetime(int(ymd[0]), int(ymd[1]), int(ymd[2])
                   ).timetuple().tm_yday  # get day of year
    doy = str(doy).zfill(3)
    if len(glob(join(dataset, '*.*o'))) == 0:  # 拼接字符，判断目录下是否存在o文件
        # get obs data
        # i = 1 if '-lax-' in dataset else 0  # use different base for LA
        for station in stas:
            fname = station + doy + '0.' + ymd[0][2:4] + 'd.gz'
            url = '/'.join([obs_url_base, ymd[0], doy, station, fname])
            try:
                reponse = requests.get(url, proxies=proxies)
                print("Status Code :", reponse.status_code)
                if reponse.status_code == 200:
                    print("normal\n")
                if reponse.content[:2] == b'\x1f\x8b':  # GZIP magic number
                    print("normal , too")
                # get obs and decompress
                obs = gzip.decompress(reponse.content)
                # write obs data
                open(join(dataset, fname[:-3]), "wb").write(obs)
            except requests.exceptions.RequestException as e:
                # 捕获所有 requests 模块相关的异常
                print(f"Request failed: {e}")
                print("Fail obs : %s" % station)
        # except:
            # try backup CORS station
            # print('Try backup CORS:', stas[i+1])
            # i += 1
            # fname = stas[i] + doy + '0.' + ymd[0][2:4] + 'd.gz'
            # url = '/'.join([obs_url_base, ymd[0], doy, stas[i], fname])
            # try:
            #     obs = gzip.decompress(requests.get(url).content) # get obs and decompress
            #     # write obs data
            #     open(join(dataset, fname[:-3]), "wb").write(obs)
            # except:
            #     print('Fail obs: %s' % dataset)

        # convert compact rinex to rinex
        crx_files = glob(join(dataset, '*.*d'))
        if len(crx_files) > 0:
            for i in crx_files:
                subprocess.call([crx2rnx_bin, '-f', i])
            conv_files = glob(join(dataset, '*.*o'))
            # ./convbin ~/Desktop/echo0820.24o -r rinex -v 3.04 -o echo0821.24o
            for i in conv_files:
                print("convert rinex version")
                j = list(i)
                j[len(i)-5] = '1'
                k = ''.join(j)
                subprocess.call(
                    [convbin, i, '-r', "rinex", '-v', "3.03", '-o', k])
                print("down")

    # get nav data
    if len(glob(join(dataset, '*.*n'))) > 0:
        continue  # file already exists
    fname = nav_file_base + doy + '0' + '.' + ymd[0][2:4] + 'n.gz'
    print(fname)
    url = '/'.join([nav_url_base, ymd[0], doy, ymd[0][2:4]+'n', fname])
    print(url)
    try:
        response = requests.get(url)
        if response.content[:2] == b'\x1f\x8b':  # GZIP magic number
            obs = gzip.decompress(response.content)
        else:
            print("Downloaded file is not a valid gzip file.")
        # obs = gzip.decompress(requests.get(url).content) # get obs and decompress
        # write nav data
        open(join(dataset, fname[:-3]), "wb").write(obs)
        print("Done nav file")
    except:
        print('Fail nav: %s' % dataset)

    # SPP process
    if(len(glob(join(dataset, '*.*n'))) > 0 and len(glob(join(dataset, '*1.*o'))) > 0):
        o_files = [os.path.abspath(f) for f in glob(join(dataset, '*1.*o'))]
        n_files = [os.path.abspath(f) for f in glob(join(dataset, '*.*n'))]
        j = 0
        try:
            for i in o_files:
                oname = stas[j]+"res"+".txt"
                pname = stas[j]+"plt"+".png"
                outfile = '/'.join([dataset, oname])
                spath = '/'.join([dataset, pname])
                subprocess.run([spp, i, n_files[0], outfile],
                               stdout=None, stderr=None)
                e_data, n_data, u_data = read_enu_data(file_path=outfile)
                print(len(e_data), len(n_data), len(u_data))
                start_time = datetime(int(ymd[0]), int(
                    ymd[1]), int(ymd[2]), 0, 0, 0)
                time_interval = timedelta(seconds=30)
                time_series = [start_time + i *
                               time_interval for i in range(2880)]
                plot_enu(time_series, e_data, n_data, u_data,
                         title=f"ENU Errors for {stas[j]}", savepath=spath)
                print(f"{i} process complete!, outfile path = {outfile}")
                j = j + 1

        except Exception as e:
            print(f"{e}")


# def batch_plot(dataset_folder, savepath, start_time):
#     time_interval = timedelta(seconds=30)
#     time_series = [start_time + i*time_interval for i in range(2880)]

#     # 假设的ENU误差数据
#     e_data = np.random.rand(len(time_series)) * 0.1  # E方向误差
#     n_data = np.random.rand(len(time_series)) * 0.2  # N方向误差
#     u_data = np.random.rand(len(time_series)) * 0.3  # U方向误差

#     # 调用绘图函数
#     plot_enu_errors(time_series, e_data, n_data, u_data, title=f"ENU Errors for {dataset_folder}", savepath)
