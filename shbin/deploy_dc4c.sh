cd $HOME/bin
deploy.sh dc4c dc4c_rserver
deploy.sh dc4c dc4c_wserver

cd $HOME/lib
deploy.sh dc4c libfasterjson.so
deploy.sh dc4c libdc4c_proto.so
deploy.sh dc4c libdc4c_util.so
deploy.sh dc4c libdc4c_api.so

