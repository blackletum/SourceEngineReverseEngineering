cd /home/ubuntu/alliedmodders/sourcemod/public/SourceEngineReverseEngineering

mkdir build
cd build
python3 ../configure.py
ambuild

cp -v package/addons/sourcemod/extensions/server_utils.ext.2.sdk2013.so ../manual_build
scp /home/ubuntu/alliedmodders/sourcemod/public/SourceEngineReverseEngineering/manual_build/server_utils.ext.2.sdk2013.so tom@gameserverhost:/home/tom/.local/share/Steam/steamapps/common/Synergy/synergy/addons/sourcemod/extensions
