
###github初使用

##为一个本地项目添加github管理
>git init

##然后把该目录下的文件都加到git本地管理中(git add .)
>git add .

##创建readme.txt,commit -m后面输入的时声明
> touch readme.txt
>git add readme.txt
>git commit -m "Wrote a readme file"

##可以创建一个名为.gitignore.的文件，然后编辑里面的内容，把不需提交的文件忽略掉！注意后面也有一个点，文件生成时后面的点会自动消失
##add除了.gitignore里面申明的文件
>git add .

##创建分支（后者创建同时会切换分支）
>git branch v1.0.3 

##或
>git checkout -b v1.0.4

##查看版本中所有分支
>git branch -a

##切换到某一分支
>git checkout v1.0.3

##删除某一分支
>git branch -D v1.0.4

##合并分支
>git merge v1.0.3

##提交到服务器
>git remote add <name> <url>
>git remote add <Android-Camera-App > <https://github.com/vincyqian/Android-Camera-App.git>
  
##把代码同步到github上
>git push












