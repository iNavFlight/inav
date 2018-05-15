# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure(2) do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://atlas.hashicorp.com/search.
  config.vm.box = "ubuntu/trusty64"
  config.vm.synced_folder ".", "/home/vagrant/inav"

  # Enable provisioning with a shell script. Additional provisioners such as
  # Puppet, Chef, Ansible, Salt, and Docker are also available. Please see the
  # documentation for more information about their specific syntax and use.
  config.vm.provision "shell", inline: <<-SHELL
	sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded
	sudo apt-add-repository ppa:brightbox/ruby-ng
	sudo apt-get update
	sudo apt-get install -y --force-yes git gcc-arm-none-eabi=4.9.3.2015q3-1trusty1 libnewlib-arm-none-eabi ruby2.4 ruby2.4-dev
  SHELL
end
