# MusicPlayer
## 音乐播放器（网易云api搜索）<br>

![image](https://github.com/user-attachments/assets/69fc95c8-45ef-40cd-a827-5687a44d562b)<br>
<br>

实现功能<br>
---
在线搜索网络歌曲播放以及歌词显示<br>
系统托盘<br>
使用sqlite3存储搜索的音乐信息和历史播放记录<br>
音乐播放器常见功能，如下一曲下一曲、暂停播放、循环/随机/单曲切换、调整音量、调整音乐播放进度<br>
自定义更换背景图片<br>

使用的网易云api<br>
---
搜索歌曲名获取ID<br>
https://music.163.com/api/search/pc?<br>
请求返回Json数据，其中包含所有相关歌曲的信息<br>
<br>
通过歌曲ID下载歌曲<br>
http://music.163.com/song/media/outer/url?<br>
请求返回mp3文件<br>
<br>
通过歌曲ID下载歌曲歌词<br>
http://music.163.com/api/song/media?<br>
请求返回Json数据，为对应ID歌曲的歌词<br>
<br>
添加下面两个请求头，下载歌曲歌词时可以跳过登录<br>
![image](https://github.com/user-attachments/assets/b598aaaa-cf0b-4278-97d4-0e2edacc4339)<br>
<br>

系统托盘功能<br>
---
![image](https://github.com/user-attachments/assets/562dac7f-ff32-48e9-ac9c-af0cf9afe7b7)<br>
<br>
左键点击播放器窗口隐藏/显示<br>
![image](https://github.com/user-attachments/assets/d3a4a4da-78e5-4d32-ac22-93b2d32b6003)<br>
右键点击弹出退出程序选项<br>
<br>



实现更换背景图片<br>
---
默认四张背景图片，以及可以用户自定义更换背景图片<br>
![image](https://github.com/user-attachments/assets/e0f356d8-eb52-4ac5-9b5b-1b4b5c5e8886)<br>
<br>
<br>
<br>
<br>
<br>
<br>
