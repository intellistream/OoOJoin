rm -rf results
rm -rf figures
cp -r build/benchmark/results .
cp -r build/benchmark/figures .
BRANCH=Figure_4p
git init
git checkout -b $BRANCH
git add .
git commit -F commit_info
git push origin $BRANCH
