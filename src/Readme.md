auto-aim/src/
├── rm_interfaces/          # 自定义消息与服务
│   ├── msg/                # Armor, Armors, Target, GimbalCmd, ...
│   └── srv/                # SetMode
├── rm_auto_aim/            # 核心算法包
│   ├── detector/           # 识别模块
│   │   ├── types.hpp       # 灯条/装甲板数据结构 + 检测参数
│   │   ├── detector.*      # 灯条检测 + 装甲板匹配
│   │   ├── pnp_solver.*    # PnP三维解算
│   │   └── armor_detector_node.*  # ROS2节点
│   └── solver/             # 解算模块
│       ├── extended_kalman_filter.*  # 10维EKF
│       ├── armor_tracker.*          # 跟踪状态机
│       ├── trajectory_compensator.hpp  # 弹道补偿
│       └── armor_solver_node.*      # ROS2节点
├── rm_hardware_driver/     # 硬件驱动
│   ├── fixed_packet.hpp    # 定长串口协议包
│   ├── serial_driver_node.*  # 串口通信节点
│   └── camera_driver_node.*  # 通用相机驱动
├── rm_bringup/             # 启动配置
│   ├── launch/bringup.launch.py  # 完整启动文件
│   └── config/node_params/       # 各节点参数YAML
└── rm_robot_description/   # 机器人URDF描述
