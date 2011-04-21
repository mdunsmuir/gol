file = File.open(ARGV[0])
outfile = File.open(ARGV[0].gsub("asp", "gol"), "w")

lines = file.readlines
y = 0
lines.each{ |line|
  next if /^!/.match(line)
  x = 0
  line.each_char{ |character|
    if character == 'O'
      outfile.puts("#{x} #{y}")
    end
    x += 1
  }
  y += 1
}

file.close
outfile.close
