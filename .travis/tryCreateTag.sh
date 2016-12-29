#!/bin/bash

export MAJOR_VER=0
export MINOR_VER=10

export GIT_BRANCH=$(git symbolic-ref $TRAVIS_COMMIT --short)
if [ "$GIT_BRANCH" == "master" ] && [ -z "$TRAVIS_TAG" ]; then
    export GIT_BUILDNUM=$(git rev-list --merges --count $TRAVIS_COMMIT)
    export GIT_TAG=v$MAJOR_VER.$MINOR_VER.$GIT_BUILDNUM
    echo "Applying tag $GIT_TAG"
    git tag $GIT_TAG -a -m "Release of version $GIT_TAG"

    API_JSON=$(printf '{"tag_name": "%s","target_commitish": "master","name": "%s","body": "Release of version %s","draft": false,"prerelease": false}' $GIT_TAG $GIT_TAG $GIT_TAG)
    curl -s -o /dev/null --data "$API_JSON" https://api.github.com/repos/Sparrow-lang/sparrow/releases?access_token=$GITHUBKEY
fi
