﻿# _**ManiaCalc**_
```
 _   .-')      ('-.         .-') _           ('-.                 ('-.                         
( '.( OO )_   ( OO ).-.    ( OO ) )         ( OO ).-.            ( OO ).-.                     
 ,--.   ,--.) / . --. /,--./ ,--,' ,-.-')   / . --. /   .-----.  / . --. / ,--.       .-----.  
 |   `.'   |  | \-.  \ |   \ |  |\ |  |OO)  | \-.  \   '  .--./  | \-.  \  |  |.-')  '  .--./  
 |         |.-'-'  |  ||    \|  | )|  |  \.-'-'  |  |  |  |('-..-'-'  |  | |  | OO ) |  |('-.  
 |  |'.'|  | \| |_.'  ||  .     |/ |  |(_/ \| |_.'  | /_) |OO  )\| |_.'  | |  |`-' |/_) |OO  ) 
 |  |   |  |  |  .-.  ||  |\    | ,|  |_.'  |  .-.  | ||  |`-'|  |  .-.  |(|  '---.'||  |`-'|  
 |  |   |  |  |  | |  ||  | \   |(_|  |     |  | |  |(_'  '--'\  |  | |  | |      |(_'  '--'\  
 `--'   `--'  `--' `--'`--'  `--'  `--'     `--' `--'   `-----'  `--' `--' `------'   `-----'  
```
**ManiaCalc** 是一个基于**IMGUI**库，专为osu!Mania玩家设计的成绩转换和计算工具。它支持 `score_v1` <-> `score_v2` 的成绩转换，并计算段位内单曲的 `acc`。

段位表目前只支持Jinjin 7k Regular & LN Dan.(2025/2/26)

开发环境: Visual Studio 2022 + IMGUI & SDL2

如果想在此基础上接着开发的话点点examples文件夹里面的sln文件就可以了 (大概?我没试过)

## 特性

- **成绩转换**：支持 `score_v1` 和 `score_v2` 之间的双向转换
- **计算单曲 `acc`**：支持计算段位内单曲的 `acc`
- **自定义段位信息**：通过 `JSON` 文件读取段位表信息，玩家可以随意修改或添加新的段位信息
## 程序介绍
点击按钮跳转所对应的界面，点击Back to Main Screen返回到主界面。

**`score_v1` <——> `score_v2`界面**:

分为左右两个窗口，左窗口负责输入，右窗口负责输出结果。
    
-----------------------------------

**Acc Calc in Dan Course界面**:

分为四象限窗口:
- 第一象限负责选择段位。
- 第二象限负责加载段位表和输入。
- 第三象限负责输出结果和debug信息 (json文件若是加载失败会在这里显示)。
- 第四象限负责显示段位各歌曲信息。
## 如何使用

在release处下载压缩包后解压就可以使用

点击任意按钮进入该界面

**`score_v1` <——> `score_v2`**:
   - 再打完一张图后，输入各个判定的数量，选择当前mod进行转换  (如果现在是score_v1那么就转score_v2,反之亦然)
```
MAX 300 200 100 50 miss
```

**Acc Calc in Dan Course**:
   - **先点击**Load Dan Pack加载段位表，然后在第一象限窗口选择段位表及其子段。
   - 在第二象限窗口输入自己在段位内的acc。(比如输入打完一首歌之后在休息段的acc，输入打完第二首歌之后在休息段的acc.以此类推输入全部acc)
   - 选择当前mod是score_v1还是v2，然后点击Calculate Song ACCs. 结果会在第三象限窗口显示。

## 注意事项 

成功加载段位表时会看见 **===End of JSON Tree===**

如果失败时会看见Failed to load dan pack,往上翻翻就会看见JSON parse error,然后从JSON文件中开始错的部分输出。**所以请注意JSON语法(^^)**

-----------------------------------


