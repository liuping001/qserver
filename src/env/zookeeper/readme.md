在一个机器部署多个zookeeper形成集群
```
cd deploy
ansible-playbook -i host_standalone.ini zoo.yml --tags="push"
ansible-playbook -i host_standalone.ini zoo.yml --tags="start"
```
在多机器部署zookeeper集群

```
cd deploy
ansible-playbook -i host_replicated.ini zoo.yml --tags="push"
ansible-playbook -i host_replicated.ini zoo.yml --tags="start"
```
根据自己的需要修改host_standalone.ini、host_replicated.ini中的配置