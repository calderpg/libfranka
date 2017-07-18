#include <gmock/gmock.h>

#include <functional>

#include <franka/robot.h>

#include "helpers.h"
#include "mock_server.h"

using ::testing::_;
using ::testing::Return;

using franka::Robot;
using franka::RobotState;
using franka::RealtimeConfig;
using franka::Stop;
using franka::Torques;
using franka::NetworkException;
using franka::IncompatibleVersionException;

using research_interface::robot::Connect;

TEST(Robot, CannotConnectIfNoServerRunning) {
  EXPECT_THROW(Robot robot("127.0.0.1"), NetworkException)
      << "Shut down local robot service to run tests.";
}

TEST(Robot, CanPerformHandshake) {
  RobotMockServer server;

  Robot robot("127.0.0.1");
  EXPECT_EQ(1, robot.serverVersion());
}

TEST(Robot, ThrowsOnIncompatibleLibraryVersion) {
  RobotMockServer server([](const Connect::Request&) {
    return Connect::Response(Connect::Status::kIncompatibleLibraryVersion);
  });

  EXPECT_THROW(Robot robot("127.0.0.1"), IncompatibleVersionException);
}

TEST(Robot, CanReadRobotState) {
  struct MockCallback {
    MOCK_METHOD1(invoke, bool(const RobotState&));
  };

  RobotMockServer server;
  Robot robot("127.0.0.1");

  server.sendEmptyState<research_interface::robot::RobotState>().spinOnce();

  MockCallback callback;
  EXPECT_CALL(callback, invoke(_));

  robot.read([&](const RobotState& robot_state) { return callback.invoke(robot_state); });
}
