#include "WPILib.h"
#include "AHRS.h"
#include <IterativeRobot.h>
#include <Joystick.h>
#include <Talon.h>
#include <LiveWindow/LiveWindow.h>
#include <RobotDrive.h>
#include <Timer.h>
#include <math.h>

class Robot: public IterativeRobot
{
	std::shared_ptr<NetworkTable> table;
	frc::Timer timer;
	Joystick stick{0}; // only joystick
    	AHRS *ahrs;
    	frc::LiveWindow* lw = frc::LiveWindow::GetInstance();
    	int autoLoopCounter;
    	int ReturningBut=2;
	frc::Talon LMotor{0};//左电机
	frc::Talon RMotor{1};//右电机
	

public:
    Robot() :

        table(NULL),
        stick(0),
        ahrs(NULL),
        lw(NULL),
        autoLoopCounter(0)
	{
		myRobot.SetExpiration(0.1);
    	timer.Start();
    }

private:
    ///地图
	double AutoMovingSpeed=0.5;
    	double CogPos[2]={0,0};
    	double CogAngle=0;
    	double BallPos[2]={0,0};
	
	double angletol=0.04;
	double angleact=0.5;
	double P_angle=0.9;//角度调整P系数
	double distol=0.01;
	
	void Move(double forward, double Rturn){
		double tmp;
		tmp=abs(forward)+abs(Rturn);
		Fcoe=abs(forward)/tmp;
		Rcoe=abs(Rturn)/tmp;
		
		RMotor.Set(forward*Fcoe-Rturn*Rcoe);
		LMotor.Set(forward*Fcoe+Rturn*Rcoe);
	}

        bool InRange(double input, double tolerate, double target){
            return ((input>=target-tolerate)&&(input<=target+tolerate));
        }


        bool MoveToPos(double XPos,double YPos, double FinalFacingAngle,double MovingSpeed){
    		double TargetAngle=0;
    		double x_dif=0;
    		double y_dif=0;
		double NowX=0;
		double NowY=0;
		
		NowX=ahrs->GetDisplacementX();
		NowY=ahrs->GetDisplacementY();
		NowAng=ahrs->GetYaw();

    		if(InRange(XPos,distol,NowX)&&(InRange(YPos,distol,NowY))){
    			if(InRange(NowAng,angletol,FinalFacingAngle)){
				Move(0,0);
    				return true;
    			}else{
    				Move(0,-(FinalFacingAngle-NowAng)*P_angle);
    			}
    		}else{
    			x_dif=NowX-XPos;
    			y_dif=NowY-YPos;
    			TargetAngle=atan(x_dif/y_dif);
    			if(InRange(NowAng,angleact,TargetAngle)){
    				Move(MovingSpeed,-(TargetAngle-NowAng)*P_angle);
    			}else{
    				Move(0.0,-(TargetAngle-NowAng)*P_angle);
    			}
    		}

    		return false;

    	}



    void RobotInit()
    {
        table = NetworkTable::GetTable("datatable");
        lw = LiveWindow::GetInstance();
        try {
              ahrs = new AHRS(SPI::Port::kMXP);
            } catch (std::exception& ex ) {
              std::string err_string = "Error instantiating navX MXP:  ";
              err_string += ex.what();
              DriverStation::ReportError(err_string.c_str());
            }
        if ( ahrs ) {
            LiveWindow::GetInstance()->AddSensor("IMU", "Gyro", ahrs);
        }
	}

    void AutonomousInit() override
    {
		timer.Reset();
		timer.Start();
    	autoLoopCounter = 0;
    }

    void AutonomousPeriodic() override
    {
        if(autoLoopCounter < 100) //Check if we've completed 100 loops (approximately 2 seconds)
        {
            autoLoopCounter++;
        }
    }

    void TeleopInit() override
    {

    }

    void TeleopPeriodic() override
    {
	//此处需要完善！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	Move(读手柄y值,读手柄x值);

    	if ( !ahrs ) return;

        bool reset_yaw_button_pressed = DriverStation::GetInstance().GetStickButton(0,1);
        if ( reset_yaw_button_pressed ) {
            ahrs->ZeroYaw();
        }

        SmartDashboard::PutBoolean( "IMU_Connected",        ahrs->IsConnected());
        SmartDashboard::PutNumber(  "IMU_Yaw",              ahrs->GetYaw());
        SmartDashboard::PutNumber(  "Displacement_X",       ahrs->GetDisplacementX() );
        SmartDashboard::PutNumber(  "Displacement_Y",       ahrs->GetDisplacementY() );

        /* Sensor Board Information                                                 */
        SmartDashboard::PutString(  "FirmwareVersion",      ahrs->GetFirmwareVersion());

        if(stick.GetRawButton(ReturningBut))MoveToPos(0,0,0,AutoMovingSpeed);

    }

    void TestPeriodic() override
    {
        lw->Run();
    }
};

START_ROBOT_CLASS(Robot);