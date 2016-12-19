$script = <<SCRIPT
sudo apt-get install make flex bison bc libpng-dev
SCRIPT

Vagrant.configure("2") do |config|
  config.vm.box = "hashicorp/precise64"
  config.vm.box_version = "1.1.0"
  config.vm.provision "shell", inline: $script
end
