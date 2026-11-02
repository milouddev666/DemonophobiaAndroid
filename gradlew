#!/bin/sh
##############################################################################
# Gradle start-up script for UN*X
##############################################################################
APP_NAME="Gradle"
APP_BASE_NAME=`basename "$0"`
APP_HOME="`pwd -P`"
CLASSPATH=$APP_HOME/gradle/wrapper/gradle-wrapper.jar

# Use JAVA_HOME if set, else find java
if [ -n "$JAVA_HOME" ] ; then
    JAVACMD="$JAVA_HOME/bin/java"
else
    JAVACMD=`which java 2>/dev/null`
fi
if [ ! -x "$JAVACMD" ] ; then
    echo "ERROR: JAVA_HOME is not set and 'java' was not found in PATH." >&2
    exit 1
fi

exec "$JAVACMD" -classpath "$CLASSPATH" \
    org.gradle.wrapper.GradleWrapperMain "$@"
