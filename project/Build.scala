import sbt._

import Keys._
import AndroidKeys._

object General {
  val settings = Defaults.defaultSettings ++ Seq (
    name := "tsynth_micro",
    version := "0.1",
    versionCode := 1,
    scalaVersion := "2.9.2",
    platformName in Android := "android-10",
    scalacOptions ++=
        Seq("-unchecked",
            "-deprecation",
            "-optimise")
  )

  val proguardSettings = Seq (
    useProguard in Android := true,
    proguardOptimizations in Android :=
        Seq(
        "-optimizationpasses 5",
        "-dontusemixedcaseclassnames",
        "-dontskipnonpubliclibraryclasses",
        "-dontpreverify",
        "-dontobfuscate",
        "-verbose",
        "-keep public class * extends android.app.Activity",
        "-keep public class * extends android.app.Application",
        "-keep public class * extends android.app.Service",
        "-keep public class * extends android.content.BroadcastReceiver",
        "-keep public class * extends android.content.ContentProvider",
        "-keep public class * extends android.app.backup.BackupAgentHelper",
        "-keep public class * extends android.preference.Preference",
        "-keepclasseswithmembernames class * { native <methods>; }",
        "-keepclasseswithmembers class * { public <init>(android.content.Context, android.util.AttributeSet); }",
        "-keepclasseswithmembers class * { public <init>(android.content.Context, android.util.AttributeSet, int); }",
        "-keepclassmembers class * extends android.app.Activity { public void *(android.view.View); }",
        "-keepclassmembers enum * { public static **[] values(); public static ** valueOf(java.lang.String); }",
        "-keep class * implements android.os.Parcelable { public static final android.os.Parcelable$Creator *; }",
        "-keep public class android.opengl.GLSurfaceView",
        "-keepclassmembers class * {** MODULE$;}",
        "-keep class scala.Array",
        "-keep class scala.Function0",
        "-keep class scala.Function1",
        "-keep class scala.Function2",
        "-keep class scala.collection.*",
        "-keep class scala.collection.immutable.*",
        "-keep class scala.collection.mutable.*",
        "-keep class scala.collection.generic.*",
        "-keep class scala.runtime.*")
  )

  lazy val fullAndroidSettings =
    General.settings ++
    AndroidProject.androidSettings ++
    TypedResources.settings ++
    proguardSettings ++
    AndroidManifestGenerator.settings ++
    AndroidMarketPublish.settings ++ Seq (
      keyalias in Android := "change-me",
      libraryDependencies += "org.scalatest" %% "scalatest" % "1.8" % "test"
    )
}

object AndroidBuild extends Build {
  lazy val main = Project (
    "tsynth_micro",
    file("."),
    settings = General.fullAndroidSettings ++
               AndroidNdk.settings
  )

  lazy val tests = Project (
    "tests",
    file("tests"),
    settings = General.settings ++
               AndroidTest.androidSettings ++
               General.proguardSettings ++ Seq (
      name := "tsynth_microTests"
    )
  ) dependsOn main
}
