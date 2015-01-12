package com.lzyblog.uninstalldemo;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

/**
 * @author liuzhiyong 转载自pengyiming
 * @note 监听此应用是否被卸载，若被卸载则弹出卸载反馈
 * @note 由于API17加入多用户支持，原有命令在4.2及更高版本上执行时缺少userSerial参数，特此修改
 * @note 此次代码修复了评论中提到的一些bug，比如清除数据、插拔USB线、覆盖安装等操作引起程序误判卸载。
 * @note 针对任何情况导致重复监听的问题，都可以通过加文件锁来防止，这比ps并读取返回结果并过滤进程名的方法要好很多。
 * @note 安装在SD卡此卸载监听依然没有问题，但是如果用户将已在Internal SD卡安装好的应用移动到external
 *       SD卡，由于.c的161行未重新files文件夹和锁文件，应该会bug，代码都有，需要的自行修复此bug即可。
 * @note 博客地址:http://lzyblog.com
 */

public class UninstalledObserverActivity extends Activity {
	/* 数据段begin */
	private static final String TAG = "UninstalledObserverActivity";
	
	//反馈网站
	private static final String WEBSITE = "http://lzyblog.com";

	// 监听进程pid
	private int mObserverProcessPid = -1;

	/* 数据段end */

	/* static */
	// 初始化监听进程
	private native int init(String userSerial, String webSite);

	static {
		Log.d(TAG, "load lib --> uninstalled_observer");
		System.loadLibrary("uninstalled_observer");
	}

	/* static */

	/* 函数段begin */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		createFile();
		
		
		// API level小于17，不需要获取userSerialNumber
		if (Build.VERSION.SDK_INT < 17) {
			mObserverProcessPid = init(null, WEBSITE);
		}
		// 否则，需要获取userSerialNumber
		else {
			mObserverProcessPid = init(getUserSerial(), WEBSITE);
		}
	}

	private void createFile() {
		File file = new File("/data/data/com.lzyblog.uninstalldemo/files/observedFile");
		if (!file.exists()) {
			try {
				File dir = new File("/data/data/com.lzyblog.uninstalldemo/files");
				if (!dir.exists()) {
					if (dir.mkdir()) {
						Log.e(TAG, "创建files目录成功");
					} else {
						Log.e(TAG, "创建files目录失败");
						return;
					}
				}
				if (file.createNewFile()) {
					Log.e(TAG, "创建observedFile成功");
					return;
				}
				Log.e(TAG, "创建observedFile失败");
			} catch (IOException e) {
				e.printStackTrace();
				Log.e(TAG, "创建observedFile失败");
			}
		} else {
			Log.e(TAG, "observedFile存在");
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();

		// 示例代码，用于结束监听进程
		// if (mObserverProcessPid > 0)
		// {
		// android.os.Process.killProcess(mObserverProcessPid);
		// }
	}

	// 由于targetSdkVersion低于17，只能通过反射获取
	private String getUserSerial() {
		Object userManager = getSystemService("user");
		if (userManager == null) {
			Log.e(TAG, "userManager not exsit !!!");
			return null;
		}

		try {
			Method myUserHandleMethod = android.os.Process.class.getMethod(
					"myUserHandle", (Class<?>[]) null);
			Object myUserHandle = myUserHandleMethod.invoke(
					android.os.Process.class, (Object[]) null);

			Method getSerialNumberForUser = userManager.getClass().getMethod(
					"getSerialNumberForUser", myUserHandle.getClass());
			long userSerial = (Long) getSerialNumberForUser.invoke(userManager,
					myUserHandle);
			return String.valueOf(userSerial);
		} catch (NoSuchMethodException e) {
			Log.e(TAG, "", e);
		} catch (IllegalArgumentException e) {
			Log.e(TAG, "", e);
		} catch (IllegalAccessException e) {
			Log.e(TAG, "", e);
		} catch (InvocationTargetException e) {
			Log.e(TAG, "", e);
		}

		return null;
	}
	/* 函数段end */
}
