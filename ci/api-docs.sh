#!/bin/bash
#
# Build an website using Doxygen and deploy to Github Pages
#
# Configuration:
#     Needs GITHUB_TOKEN in environment, usually using something like
#         env:
#           GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
#
set -xe

# Some hard-coded assumptions:
readonly SITE_DIR='docs'           # Doxygen final output directory
readonly GP_BRANCH='gh-pages'      # Branch used as input by Github Pages

# Install doxygen
sudo apt-get install -y doxygen

# Build site and block github's default jekyll formatting
mkdir docs
cd manual; doxygen 2>doxy-errors.txt; cd ..
mv manual/api-docs manual/doxy-errors.txt $SITE_DIR
touch $SITE_DIR/.nojekyll

# Set up a git environment in $SITE_DIR
author_email=$(git log -1 --pretty=format:"%ae")
cd $SITE_DIR
git init -b pages
git remote add origin \
    https://$GITHUB_ACTOR:$GITHUB_TOKEN@github.com/$GITHUB_REPOSITORY
git config --local user.email "$author_email"
git config --local user.name "$GITHUB_ACTOR"

# Commit changes and push to $GP_BRANCH
git add --all .
git commit -q -m "[CI] Updating $GP_BRANCH branch from ${GITHUB_SHA:0:8}"
git push -f origin pages:$GP_BRANCH
