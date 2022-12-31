BRANCH=backup_arrivaltime_discard
git init
git checkout -b $BRANCH
git add .
git commit -F commit_info
git push origin $BRANCH
