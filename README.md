项目结构
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

编译运行指令
cd ~/auto-aim
colcon build --symlink-install
source install/setup.bash
ros2 launch rm_bringup bringup.launch.py

数据流
Camera → /image_raw → ArmorDetector → /detector/armors → ArmorSolver → /solver/gimbal_cmd → Serial
                        (灯条检测      (EKF跟踪+预测
                         装甲板匹配     弹道补偿
                         PnP解算)       反陀螺策略)

核心模块说明
模块	     功能
灯条检测	     灰度→二值化→轮廓→旋转矩形→长宽比/角度/颜色筛选
装甲板匹配    灯条对间距比、连线角度、长度比约束 + 包含检查
PnP解算	     solvePnP(IPPE_SQUARE) 双解 + 重投影误差择优
EKF跟踪	     10维状态(旋转中心+速度+yaw+角速度+半径), 数值雅可比
状态机	     LOST→DETECTING→TRACKING↔TEMP_LOST(掉帧处理)
弹道补偿	     抛物线+空气阻力模型, 迭代求解发射仰角
反陀螺	     角速度判断 → 选择正对相机的装甲板瞄准
串口通信	     定长包[0xFF][data][XOR校验][0x0D], 1kHz发送


根据实际相机标定结果修改 camera_driver_params.yaml 中的内参矩阵和畸变系数
