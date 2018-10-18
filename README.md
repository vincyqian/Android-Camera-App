  <h2>为一个本地项目添加github管理</h2>


  <p>git init</p>
  
  <h2>然后把该目录下的文件都加到git本地管理中(git add .)</h2>
  
  <p>git add .</p>
  
  <h2>创建readme.txt,commit -m后面输入的时声明</h2>
  
  <p>touch readme.txt
  git add readme.txt
  git commit -m "Wrote a readme file"</p>
  
  <h2>可以创建一个名为.gitignore.的文件，然后编辑里面的内容，把不需提交的文件忽略掉！注意后面也有一个点，文件生成时后面的点会自动消失</h2>


 <h2>add除了.gitignore里面申明的文件</h2>


  <p>git add .</p>
  
  <h2>创建分支（后者创建同时会切换分支）</h2>
  
  <p>git branch v1.0.3 </p>
  
  <h2>或</h2>
  
  <p>git checkout -b v1.0.4</p>
  
  <h2>查看版本中所有分支</h2>
  
  <p>git branch -a</p>
  
  <h2>切换到某一分支</h2>
  
  <p>git checkout v1.0.3</p>
  
  <h2>删除某一分支</h2>
  
  <p>git branch -D v1.0.4</p>
  
  <h2>合并分支</h2>
  
  <p>git merge v1.0.3</p>
  
  <h2>提交到服务器</h2>
  
  <p>git remote add <name> <url>
  git remote add <Android-Camera-App > <a href="https://github.com/vincyqian/Android-Camera-App.git">https://github.com/vincyqian/Android-Camera-App.git</a></p>
  
  <h2>把代码同步到github上</h2>
  
  <p>git push</p>
</blockquote>
