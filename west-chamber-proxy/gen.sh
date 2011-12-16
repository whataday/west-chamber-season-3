curl https://raw.github.com/liruqi/kernet/stable/kerdns/dnsmasq.conf|awk 'BEGIN {FS="/"} /^address=/ {print $3" "$2}' |grep -v "127\.0\.0\.1" > ghosts
echo '203.208.46.30    www.youtube.com' >> ghosts
echo '203.208.46.30    *.ytimg.com' >> ghosts
