BRANCH=stable_torch_and_yuanzhen
git init
git checkout -b $BRANCH
git add .
git commit -F commit_info
git push origin $BRANCH
