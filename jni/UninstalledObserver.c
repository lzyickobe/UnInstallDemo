
/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * NOTICE:这份c文件有两处需要改动，分别标识了 TODO1 、 TODO2
 */
#include<jni.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/inotify.h>
#include<sys/stat.h>

#include<android/log.h>

/* 宏定义begin */
//清0宏
#define MEM_ZERO(pDest, destSize) memset(pDest, 0, destSize)

//LOG宏定义
#define LOG_INFO(tag, msg) __android_log_write(ANDROID_LOG_INFO, tag, msg)
#define LOG_DEBUG(tag, msg) __android_log_write(ANDROID_LOG_DEBUG, tag, msg)
#define LOG_WARN(tag, msg) __android_log_write(ANDROID_LOG_WARN, tag, msg)
#define LOG_ERROR(tag, msg) __android_log_write(ANDROID_LOG_ERROR, tag, msg)


/* 内全局变量begin */
static char TAG[] = "UninstalledObserverActivity.init";
static jboolean isCopy = JNI_TRUE;

//--TODO1--------------------以下需要改成自己apk的包名------------------------------------
static const char APP_DIR[] = "/data/data/com.lzyblog.uninstalldemo/lib";
static const char APP_FILES_DIR[] = "/data/data/com.lzyblog.uninstalldemo/files";
static const char APP_OBSERVED_FILE[] = "/data/data/com.lzyblog.uninstalldemo/files/observedFile";
static const char APP_LOCK_FILE[] = "/data/data/com.lzyblog.uninstalldemo/files/lockFile";
/* 内全局变量 */

/*-------TODO2-----------------------
 * Class:     需要改成自己app的类名
 * Method:    init
 * Signature: ()V
 * return: 子进程pid
 */
JNIEXPORT int JNICALL Java_com_lzyblog_uninstalldemo_UninstalledObserverActivity_init(JNIEnv *env, jobject obj, jstring userSerial, jstring website)
{
    jstring tag = (*env)->NewStringUTF(env, TAG);
	char* websiteStr=(char*) (*env)->GetStringUTFChars(env, website, NULL);

    LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
            , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "init observer"), &isCopy));

    // fork子进程，以执行轮询任务
    pid_t pid = fork();
    if (pid < 0)
    {
        LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
                , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "fork failed !!!"), &isCopy));

        exit(1);
    }
    else if (pid == 0)
    {
	    LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
	            , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "fork Success !!!"), &isCopy));
        // 若监听文件所在文件夹不存在，创建
        FILE *p_filesDir = fopen(APP_FILES_DIR, "r");
        if (p_filesDir == NULL)
        {
            int filesDirRet = mkdir(APP_FILES_DIR, S_IRWXU | S_IRWXG | S_IXOTH);
            if (filesDirRet == -1)
            {
                LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
                        , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "mkdir failed !!!"), &isCopy));

                exit(1);
            }
            LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
	            , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "mkdir Success !!!"), &isCopy));
        }
        LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
	            , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "app dir is exist !!!"), &isCopy));
        

        // 若被监听文件不存在，创建文件
        FILE *p_observedFile = fopen(APP_OBSERVED_FILE, "r");
        if (p_observedFile == NULL)
        {
         LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
	            , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "the observed file is not existed!!!"), &isCopy));
            p_observedFile = fopen(APP_OBSERVED_FILE, "w");
            if (p_observedFile == NULL) {
            LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
	            , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "the observed file create falied!!!"), &isCopy));
            }
            
        }
        fclose(p_observedFile);

        // 创建锁文件，通过检测加锁状态来保证只有一个卸载监听进程
        int lockFileDescriptor = open(APP_LOCK_FILE, O_RDONLY);
        if (lockFileDescriptor == -1)
        {
            lockFileDescriptor = open(APP_LOCK_FILE, O_CREAT);
        }
        int lockRet = flock(lockFileDescriptor, LOCK_EX | LOCK_NB);
        if (lockRet == -1)
        {
            LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "observed by another process"), &isCopy));

            exit(0);
        }
        LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "observed by child process"), &isCopy));

        // 分配空间，以便读取event
        void *p_buf = malloc(sizeof(struct inotify_event));
        if (p_buf == NULL)
        {
            LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "malloc failed !!!"), &isCopy));

            exit(1);
        }
        // 分配空间，以便打印mask
        int maskStrLength = 7 + 10 + 1;// mask=0x占7字节，32位整形数最大为10位，转换为字符串占10字节，'\0'占1字节
        char *p_maskStr = malloc(maskStrLength);
        if (p_maskStr == NULL)
        {
            free(p_buf);

            LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "malloc failed !!!"), &isCopy));

            exit(1);
        }

        // 开始监听
        LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
                , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "start observe"), &isCopy));

        // 初始化
        int fileDescriptor = inotify_init();
        if (fileDescriptor < 0)
        {
            free(p_buf);
            free(p_maskStr);

            LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "inotify_init failed !!!"), &isCopy));

            exit(1);
        }

        // 添加被监听文件到监听列表
        int watchDescriptor = inotify_add_watch(fileDescriptor, APP_OBSERVED_FILE, IN_ALL_EVENTS);
        if (watchDescriptor < 0)
        {
            free(p_buf);
            free(p_maskStr);

            LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "inotify_add_watch failed !!!"), &isCopy));

            exit(1);
        }

        while(1)
        {
            // read会阻塞进程
            size_t readBytes = read(fileDescriptor, p_buf, sizeof(struct inotify_event));

            // 打印mask
            snprintf(p_maskStr, maskStrLength, "mask=0x%x\0", ((struct inotify_event *) p_buf)->mask);
            LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, p_maskStr), &isCopy));

            if (IN_DELETE_SELF == ((struct inotify_event *) p_buf)->mask)
            {
            	LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "the observer file is deleted"), &isCopy));
                
                 //inotify_rm_watch(fileDescriptor, watchDescriptor);
                 //break;
                 
                //实际判断是有问题的，不能确定哪个是卸载，哪个是清除数据，先屏蔽
                
                
                FILE *p_appDir = fopen(APP_DIR, "r");
                // 确认已卸载
                if (p_appDir == NULL)
                {
	                LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
	                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "app is uninstall"), &isCopy));
                    inotify_rm_watch(fileDescriptor, watchDescriptor);

                    break;
                }
                // 未卸载，可能用户执行了"清除数据"
                else
                {
	                LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
	                    , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "app not uninstall , the data is empty"), &isCopy));
	                    fclose(p_appDir);
	
	                    // 重新创建被监听文件，并重新监听
	                    FILE *p_observedFile = fopen(APP_OBSERVED_FILE, "w");
	                    fclose(p_observedFile);
	
	                    int watchDescriptor = inotify_add_watch(fileDescriptor, APP_OBSERVED_FILE, IN_ALL_EVENTS);
	                    if (watchDescriptor < 0)
	                    {
	                        free(p_buf);
	                        free(p_maskStr);
	
	                        LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
	                                , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "inotify_add_watch failed !!!"), &isCopy));
	
	                        exit(1);
                    	}
                }
                
            }
        }

        // 释放资源
        free(p_buf);
        free(p_maskStr);

        // 停止监听
        LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &isCopy)
                , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "stop observe"), &isCopy));

        if (userSerial == NULL)
        {
            // 执行命令am start -a android.intent.action.VIEW -d $(url)
            execlp("am", "am", "start", "-a", "android.intent.action.VIEW", "-d", websiteStr, (char *)NULL);
        }
        else
        {
            // 执行命令am start --user userSerial -a android.intent.action.VIEW -d $(url)
            execlp("am", "am", "start", "--user", (*env)->GetStringUTFChars(env, userSerial, &isCopy), "-a", "android.intent.action.VIEW", "-d", websiteStr, (char *)NULL);
        }

        // 执行命令失败log
        LOG_ERROR((*env)->GetStringUTFChars(env, tag, &isCopy)
                , (*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "exec AM command failed !!!"), &isCopy));
		(*env)->ReleaseStringUTFChars(env, website, websiteStr);
    }
    else
    {
        // 父进程直接退出，使子进程被init进程领养，以避免子进程僵死，同时返回子进程pid
		(*env)->ReleaseStringUTFChars(env, website, websiteStr);
        return pid;
    }
}
