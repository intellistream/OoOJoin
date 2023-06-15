BRANCH=zipf_sel_bug_fix
git init
git checkout -b $BRANCH
git add .
git commit -F commit_info
git push origin $BRANCH
