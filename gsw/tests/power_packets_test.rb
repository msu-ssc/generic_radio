require 'cosmos'
require 'cosmos/packets/packet_config'
require 'cosmos/packets/commands'
module CosmosCfsConfig
  PROCESSOR_ENDIAN = 'LITTLE_ENDIAN'
end
config = Cosmos::PacketConfig.new
base = File.expand_path('../GENERIC_RADIO/cmd_tlm', __dir__)
config.process_file(File.join(base, 'GENERIC_RADIO_CMD.txt'), 'GENERIC_RADIO')
config.process_file(File.join(base, 'GENERIC_RADIO_TLM.txt'), 'GENERIC_RADIO')
commands = Cosmos::Commands.new(config)
[0, 50, 100].each do |power|
  packet = commands.build_cmd('GENERIC_RADIO', 'IRIS_RADIO_SET_POWER', {'MILLIWATTS' => power})
  expected = [0x1930, 0xC000, 5, 4, 0].pack('nnnCC') + [power].pack('V')
  raise 'Wrong command bytes' unless packet.buffer == expected
end
begin
  commands.build_cmd('GENERIC_RADIO', 'IRIS_RADIO_SET_POWER', {'MILLIWATTS' => 101})
  raise 'COSMOS allowed power above 100'
rescue RuntimeError => error
  raise unless error.message.include?("not in valid range")
end
packet = config.telemetry['GENERIC_RADIO']['GENERIC_RADIO_HK_TLM'].clone
bytes = [0x0930, 0xC000, 31].pack('nnn') + "\x00" * 10
bytes += [1, 2, 3, 4, 5].pack('C*') + [6, 7, 8, 50].pack('V*') + [1].pack('C')
packet.buffer = bytes
raise 'Wrong packet size' unless packet.length == 38
raise 'Wrong power offset' unless packet.read('TRANSMIT_POWER_MW') == 50
raise 'Wrong validity offset' unless packet.read('POWER_VALID') == 1
puts 'COSMOS command bytes, range checks, and telemetry decoding passed'
