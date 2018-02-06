#!/bin/bash

if [ $TRAVIS_OS_NAME == linux ]; then
    echo "---------- Performance testing ----------"

    # The name of the build - take the travis build number
    BUILD_NAME=$TRAVIS_BUILD_NUMBER

    # Check out performance monitoring repo from GitHub
    git clone https://github.com/Sparrow-lang/perf-monitoring.git _perf-monitoring


    # Run the performance test, storing the result in the perf-monitor data folder
    echo "Running performance tests"
    cd tests/
    python perf-test.py $BUILD_NAME -b $TRAVIS_BRANCH > "../_perf-monitoring/_data/builds/$BUILD_NAME.yaml"

    # Print the measurements, for reference
    cat "../_perf-monitoring/_data/builds/$BUILD_NAME.yaml"

    # Chain with existing builds and process the results
    echo "Integrating performance measurements"
    cd ../_perf-monitoring/
    python _scripts/genPerfTrends.py $BUILD_NAME

    # Add this build to the list of recent builds
    python _scripts/addRecent.py $BUILD_NAME


    # Commit back to GitHub, to be displayed on the perf monitoring site
    if [ -n "$PERFMONITORING_KEY" ]; then
        git add .
        git -c user.name='travis' -c user.email='travis' commit -m  "Performance for build: $BUILD_NAME"

        # the output quiet, so that we don't leak the API token
        echo "Pushing to GitHub"
        git remote add origin https://$PERFMONITORING_KEY@github.com/Sparrow-lang/perf-monitoring.git > /dev/null 2>&1
        git push --quiet --force --set-upstream origin master
    fi
fi
