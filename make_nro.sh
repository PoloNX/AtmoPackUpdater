 cd amssu-rcm
 make
 cd ..
 cd amssu-forwarder
 make
 cd ..
 cp amssu-rcm/output/ams_rcm.bin resources/payload/ams_rcm.bin
 cp amssu-forwarder/amssu-forwarder.nro resources/forwarder/amssu-forwarder.nro
 make
