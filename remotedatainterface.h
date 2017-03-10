/*********************************************************************************
**
** Copyright (c) 2017 The University of Notre Dame
** Copyright (c) 2017 The Regents of the University of California
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice, this
** list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above copyright notice, this
** list of conditions and the following disclaimer in the documentation and/or other
** materials provided with the distribution.
**
** 3. Neither the name of the copyright holder nor the names of its contributors may
** be used to endorse or promote products derived from this software without specific
** prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
** EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
** SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
** BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
** IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
**
***********************************************************************************/

// Contributors:
// Written by Peter Sempolinski, for the Natural Hazard Modeling Laboratory, director: Ahsan Kareem, at Notre Dame

#ifndef REMOTEDATAINTERFACE_H
#define REMOTEDATAINTERFACE_H

#include <QObject>
#include <QList>
#include <QString>

enum class FileType {FILE, DIR, SIM_LINK, INVALID}; //Add more as needed

class FileMetaData
{
public:
    FileMetaData();
    FileMetaData(FileMetaData * copyFrom);
    FileMetaData operator=(const FileMetaData & toCopy);

    void setFullFilePath(QString fullPath);
    void setSize(int newSize);
    void setType(FileType newType);

    QString getFullPath();
    QString getFileName();
    QString getContainingPath();
    int getSize();
    FileType getFileType();

private:
    //Add more members as needed, all must have reasonable defaults, and be handled in copy constructor
    QString fullContainingPath; //ie. full path without this files own name
    QString fileName;
    int fileSize = 0; //in bytes?
    FileType myType = FileType::INVALID;
};

//Good means the request was good and
//Fail means the remote service replied, but did not like the request, for some reason
//No Connect means that the request did not get thru to the remote service at all
enum class RequestState {FAIL, GOOD, NO_CONNECT};

class RemoteDataReply : public QObject
{
    Q_OBJECT

public:
    //The RemoteDataInterface is never to return NULL for a RemoteDataReply.
    //If the request is not valid (nothing could be sent) the function below returns false.
    virtual bool isValidReply() = 0;

    virtual QString getErrorMessage() = 0;

signals:
    virtual void haveCurrentRemoteDir(RequestState cmdReply, QString * pwd);
    virtual void connectionsClosed(RequestState cmdReply);

    virtual void haveAuthReply(RequestState authReply);
    virtual void haveLSReply(RequestState cmdReply, QList<FileMetaData> * fileDataList);

    virtual void haveDeleteReply(RequestState replyState, QString * oldFilePath);
    virtual void haveMoveReply(RequestState authReply, QString * oldFilePath, FileMetaData * revisedFileData);
    virtual void haveCopyReply(RequestState authReply, FileMetaData * newFileData);
    virtual void haveRenameReply(RequestState replyState, QString * oldFilePath, FileMetaData * newFileData);

    virtual void haveMkdirReply(RequestState authReply, FileMetaData * newFolderData);

    virtual void haveUploadReply(RequestState authReply, FileMetaData * newFileData);
    virtual void haveDownloadReply(RequestState authReply, QString * localDest);

    //Job replys should be in an intelligble format, JSON is used by Agave and AWS for various things
    virtual void haveJobReply(RequestState authReply, QJsonDocument * rawJobReply);
};

class RemoteDataInterface : public QObject
{
    Q_OBJECT

public:
    //Defaults to directory root,
    //Subsequent commands with remote folder names are either absolute paths
    //or reletive to the current working directory
    virtual RemoteDataReply * setCurrentRemoteWorkingDirectory(QString cd) = 0;
    virtual RemoteDataReply * closeAllConnections() = 0;

    //Remote tasks to be implemented in subclasses:
    //Returns a RemoteDataReply, which should have the correct signal attached to an appropriate slot
    virtual RemoteDataReply * performAuth(QString uname, QString passwd) = 0;

    virtual RemoteDataReply * remoteLS(QString dirPath) = 0;

    virtual RemoteDataReply * deleteFile(QString toDelete) = 0;
    virtual RemoteDataReply * moveFile(QString from, QString to) = 0;
    virtual RemoteDataReply * copyFile(QString from, QString to) = 0;
    virtual RemoteDataReply * renameFile(QString fullName, QString newName) = 0;

    virtual RemoteDataReply * mkRemoteDir(QString loc, QString newName) = 0;

    virtual RemoteDataReply * uploadFile(QString loc, QString localFileName) = 0;
    virtual RemoteDataReply * downloadFile(QString localDest, QString remoteName) = 0;

    virtual RemoteDataReply * runRemoteJob(QString jobName, QString jobParameters, QString remoteWorkingDir) = 0;
};

#endif // REMOTEDATAINTERFACE_H