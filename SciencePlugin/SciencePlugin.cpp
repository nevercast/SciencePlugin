#include "bakkesmod\wrappers\GameEvent\TutorialWrapper.h"
#include "bakkesmod\wrappers\GameObject\CarWrapper.h"
#include "bakkesmod\wrappers\GameObject\BallWrapper.h"
#include "bakkesmod\wrappers\arraywrapper.h"
#include "bakkesmod\wrappers\CVarWrapper.h"
#include "bakkesmod\wrappers\CVarManagerWrapper.h"
#include "utils\parser.h"

#include "SciencePlugin.h"
#include "HelperFunctions.h"

BAKKESMOD_PLUGIN(SciencePlugin, "Science plugin", "1.0", PLUGINTYPE_FREEPLAY)

using namespace sp;

void SciencePlugin::onLoad()
{
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", std::bind(&SciencePlugin::OnViewportTick, this));
	logger.cvarManager = this->cvarManager;
	last_time = std::chrono::high_resolution_clock::now();
	start_time = last_time;	
}

void SciencePlugin::OpenLog(std::string greeting)
{
	if (!this->dump_file.is_open())
	{
		this->dump_file.open("dump.txt", std::ios::out);
		this->dump_file << greeting + "\n";
		cvarManager->log("Dump file opened");
	}
}

void SciencePlugin::CloseLog() {
	if (this->dump_file.is_open())
	{
		this->dump_file.close();
		cvarManager->log("dump file closed");
	}
}

void SciencePlugin::onUnLoad()
{
	this->CloseLog();
}


void SciencePlugin::OnViewportTick()
{
	if (gameWrapper->IsInOnlineGame() || !(gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining() || gameWrapper->IsInGame())) {
		this->CloseLog();
		return;
	}

	auto now = std::chrono::high_resolution_clock::now();
	auto elapsed = now - last_time;
	auto elapsed_total = now - start_time;
	last_time = now;
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	std::string microseconds_str = std::to_string(microseconds);
	long long timestamp = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_total).count();
	std::string timestamp_str = std::to_string(timestamp);

	this->OpenLog("Log opened at " + microseconds_str);
			
	ServerWrapper game = gameWrapper->GetGameEventAsServer();

	BallWrapper ball = game.GetBall();
	if (!ball.IsNull())
	{
		auto ballRBstate = ball.GetRBState();
		auto ballLocRB = ballRBstate.Location;
		auto ballLinVelRB = ballRBstate.LinearVelocity;
		auto ballQuatRB = ballRBstate.Quaternion;
		auto ballAngVelRB = ballRBstate.AngularVelocity;

		if (dump_file.is_open())
		{
			auto output_line = microseconds_str
				+ "b, "
				+ ", " + vector_to_string(ballLocRB, 2)
				+ ", " + vector_to_string(ballLinVelRB, 2)
				+ ", " + quat_to_string(ballQuatRB)
				+ ", " + vector_to_string(ballAngVelRB, 4)
				;
			dump_file << output_line + "\n";
		}
	}

	auto car = game.GetGameCar();

	auto carInput = car.GetInput();
	if (dump_file.is_open())
	{
		auto output_line = microseconds_str
			+ "c, "
			+ ", " + std::to_string(carInput.ActivateBoost)
			+ ", " + to_string_decimals(carInput.DodgeForward, 2)
			+ ", " + to_string_decimals(carInput.DodgeStrafe, 2)
			+ ", " + std::to_string(carInput.Handbrake)
			+ ", " + std::to_string(carInput.HoldingBoost)
			+ ", " + std::to_string(carInput.Jump)
			+ ", " + std::to_string(carInput.Jumped)
			+ ", " + to_string_decimals(carInput.Pitch, 2)
			+ ", " + to_string_decimals(carInput.Roll, 2)
			+ ", " + to_string_decimals(carInput.Steer, 2)
			+ ", " + to_string_decimals(carInput.Throttle, 2)
			+ ", " + to_string_decimals(carInput.Yaw, 2)
			;
		dump_file << output_line + "\n";
	}

	auto rbstate = car.GetRBState();
	auto locRB = rbstate.Location;
	auto linVelRB = rbstate.LinearVelocity;
	auto horVel = Vector(linVelRB.X, linVelRB.Y, 0);
	auto quatRB = rbstate.Quaternion;
	auto angVelRB = rbstate.AngularVelocity;

	Vector up = quatToUp(quatRB);
	Vector right = quatToRight(quatRB);
	Vector fwd = quatToFwd(quatRB);

	auto linLocalFwd = Vector::dot(linVelRB, fwd);
	auto linLocalRight = Vector::dot(linVelRB, right);
	auto linLocalUp = Vector::dot(linVelRB, up);

	Vector linLocal = Vector(linLocalFwd, linLocalRight, linLocalUp);

	auto lonSpeed = Vector::dot(horVel, fwd);
	auto latSpeed = Vector::dot(horVel, right);

	if (dump_file.is_open())
	{
		auto output_line = microseconds_str
			+ "p, "
			+ ", " + vector_to_string(locRB, 2)
			+ ", " + vector_to_string(linVelRB, 2)
			+ ", " + quat_to_string(quatRB, 20)
			+ ", " + vector_to_string(angVelRB, 4)
			;
		dump_file << output_line + "\n";
	}
		
}