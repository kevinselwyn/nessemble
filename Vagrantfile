Vagrant.configure("2") do |config|
  config.vm.provision :shell, path: "provision/provision.sh"

  config.vm.define "linux64", primary: true do |linux64|
    linux64.vm.box = "hashicorp/precise64"
    linux64.vm.box_version = "1.1.0"
  end

  config.vm.define "linux32" do |linux32|
      linux32.vm.box = "hashicorp/precise32"
      linux32.vm.box_version = "1.0.0"
  end
end
