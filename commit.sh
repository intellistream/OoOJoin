#!/bin/bash
BRANCH=gh-pages
git checkout -b ${BRANCH}
git init 
git add .
#git rm -r --cached doc
git commit -F "commit_notes"
git push origin ${BRANCH}
