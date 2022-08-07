 cd amssu-rcm
 make
 cd ..
 cd amssu-forwarder
 make
 cd ..
 copy amssu-rcm/output/ams_rcm.bin resources/payload/ams_rcm.bin
 copy amssu-forwarder/amssu-forwarder.nro resources/forwarder/amssu-forwarder.nro
 make
