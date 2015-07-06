#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libthreadimageio_SRCS
    filereadwritelock.cpp
    loadsavethread.cpp
    managedloadsavethread.cpp
    sharedloadsavethread.cpp
    loadingdescription.cpp
    loadingcache.cpp
    loadingcacheinterface.cpp
    loadsavetask.cpp
    previewloadthread.cpp
    previewtask.cpp
    previewsettings.cpp
    thumbnailbasic.cpp
    thumbnailcreator.cpp
    thumbnailloadthread.cpp
    thumbnailtask.cpp
    thumbnailsize.cpp
)

add_library(threadimageio_src OBJECT ${libthreadimageio_SRCS})