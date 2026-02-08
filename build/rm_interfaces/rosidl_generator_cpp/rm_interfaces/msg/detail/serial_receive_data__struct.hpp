// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from rm_interfaces:msg/SerialReceiveData.idl
// generated code does not contain a copyright notice

#ifndef RM_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_HPP_
#define RM_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__rm_interfaces__msg__SerialReceiveData __attribute__((deprecated))
#else
# define DEPRECATED__rm_interfaces__msg__SerialReceiveData __declspec(deprecated)
#endif

namespace rm_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct SerialReceiveData_
{
  using Type = SerialReceiveData_<ContainerAllocator>;

  explicit SerialReceiveData_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->bullet_speed = 0.0;
      this->cur_yaw = 0.0;
      this->cur_pitch = 0.0;
      this->color = 0;
      this->mode = 0;
    }
  }

  explicit SerialReceiveData_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->bullet_speed = 0.0;
      this->cur_yaw = 0.0;
      this->cur_pitch = 0.0;
      this->color = 0;
      this->mode = 0;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _bullet_speed_type =
    double;
  _bullet_speed_type bullet_speed;
  using _cur_yaw_type =
    double;
  _cur_yaw_type cur_yaw;
  using _cur_pitch_type =
    double;
  _cur_pitch_type cur_pitch;
  using _color_type =
    uint8_t;
  _color_type color;
  using _mode_type =
    uint8_t;
  _mode_type mode;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__bullet_speed(
    const double & _arg)
  {
    this->bullet_speed = _arg;
    return *this;
  }
  Type & set__cur_yaw(
    const double & _arg)
  {
    this->cur_yaw = _arg;
    return *this;
  }
  Type & set__cur_pitch(
    const double & _arg)
  {
    this->cur_pitch = _arg;
    return *this;
  }
  Type & set__color(
    const uint8_t & _arg)
  {
    this->color = _arg;
    return *this;
  }
  Type & set__mode(
    const uint8_t & _arg)
  {
    this->mode = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    rm_interfaces::msg::SerialReceiveData_<ContainerAllocator> *;
  using ConstRawPtr =
    const rm_interfaces::msg::SerialReceiveData_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      rm_interfaces::msg::SerialReceiveData_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      rm_interfaces::msg::SerialReceiveData_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__rm_interfaces__msg__SerialReceiveData
    std::shared_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__rm_interfaces__msg__SerialReceiveData
    std::shared_ptr<rm_interfaces::msg::SerialReceiveData_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const SerialReceiveData_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->bullet_speed != other.bullet_speed) {
      return false;
    }
    if (this->cur_yaw != other.cur_yaw) {
      return false;
    }
    if (this->cur_pitch != other.cur_pitch) {
      return false;
    }
    if (this->color != other.color) {
      return false;
    }
    if (this->mode != other.mode) {
      return false;
    }
    return true;
  }
  bool operator!=(const SerialReceiveData_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct SerialReceiveData_

// alias to use template instance with default allocator
using SerialReceiveData =
  rm_interfaces::msg::SerialReceiveData_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace rm_interfaces

#endif  // RM_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_HPP_
