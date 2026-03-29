#include<stdio.h>
#include<easyx.h>
#include <graphics.h>
#include <tchar.h> //结束文本
#include <random>  // C++11 随机数库
#include <ctime>   // 包含 time()
#include <math.h> //banzai图案

IMAGE img_background;//背景

IMAGE rainstone;
IMAGE rainstone_mask;
IMAGE lineplain;
IMAGE lineplain_mask;
IMAGE BULLET;
IMAGE BULLET_mask;
IMAGE banzaiplain;
#define PI 3.14159265358979323846//banzai设定

struct Mouse
{
	int x;
	int y;
	int live;
}mouse;  // 只需要一个鼠标位置，不需要数组

// 计算线段与正方形边界的交点
void getSquareIntersection(int centerX, int centerY, int halfSide, double angle, int* outX, int* outY)
{
	// 方向向量
	double dx = cos(angle);
	double dy = sin(angle);

	// 计算与四条边的交点参数 t
	double t;
	double minT = 1e10;  // 初始化为一个大值

	// 与右边 (x = centerX + halfSide) 的交点
	if (dx > 0)  // 方向向右
	{
		t = (halfSide) / dx;
		int y = centerY + (int)(t * dy);
		if (y >= centerY - halfSide && y <= centerY + halfSide)
		{
			if (t < minT)
			{
				minT = t;
				*outX = centerX + halfSide;
				*outY = y;
			}
		}
	}

	// 与左边 (x = centerX - halfSide) 的交点
	if (dx < 0)  // 方向向左
	{
		t = (-halfSide) / dx;
		int y = centerY + (int)(t * dy);
		if (y >= centerY - halfSide && y <= centerY + halfSide)
		{
			if (t < minT)
			{
				minT = t;
				*outX = centerX - halfSide;
				*outY = y;
			}
		}
	}

	// 与下边 (y = centerY + halfSide) 的交点
	if (dy > 0)  // 方向向下
	{
		t = (halfSide) / dy;
		int x = centerX + (int)(t * dx);
		if (x >= centerX - halfSide && x <= centerX + halfSide)
		{
			if (t < minT)
			{
				minT = t;
				*outX = x;
				*outY = centerY + halfSide;
			}
		}
	}

	// 与上边 (y = centerY - halfSide) 的交点
	if (dy < 0)  // 方向向上
	{
		t = (-halfSide) / dy;
		int x = centerX + (int)(t * dx);
		if (x >= centerX - halfSide && x <= centerX + halfSide)
		{
			if (t < minT)
			{
				minT = t;
				*outX = x;
				*outY = centerY - halfSide;
			}
		}
	}
}
//定义banzai图案
//子弹定义 上
enum My//+
{
	BULLET_NUM = 20,
	STONE_NUM = 1000
};
struct Plant//+
{
	int x;
	int y;
	bool live;
	bool Bulletlast[STONE_NUM] = { 0 };//穿透，检测上一帧是否触碰
	int Bullettime=1;
}bull[BULLET_NUM];
struct Rainstone//+
{
	int x;
	int y;
	bool live;
	int blood;

}rain[STONE_NUM];
struct mousemove
{
	int x = 400;
	int y = 600;
	int live = 1;
	bool banzai = 0;
	int banzaitime = 0;
}Mouse;

//子弹消息
ExMessage msg = { 0 };

//菜单及相关设置

struct Box {
	int startX[4] = { 0 };   // 每个按钮的左上X
	int startY[4] = { 0 };   // 每个按钮的左上Y
	int endX[4] = { 0 };     // 每个按钮的右下X
	int endY[4] = { 0 };     // 每个按钮的右下Y
	int Index = 0;
} box;
void recordBox(int index, int x1, int y1, int x2, int y2)
{
	box.startX[index] = x1;
	box.startY[index] = y1;
	box.endX[index] = x2;
	box.endY[index] = y2;
}
// 绘制带框文字的函数
void drawTextWithBox(int x, int y, LPCTSTR text, int fontSize,
	COLORREF textColor, COLORREF borderColor, int padding)
{
	// 保存当前的背景色，以便后续恢复
	COLORREF oldBkColor = getbkcolor();

	// 设置字体和文字颜色
	settextstyle(fontSize, 0, _T("楷体"));
	settextcolor(textColor);

	// 获取文字尺寸
	int textWidth = textwidth(text);
	int textHeight = textheight(text);

	// 计算框的尺寸
	int boxWidth = textWidth + padding * 2;
	int boxHeight = textHeight + padding * 2;

	// 绘制白色填充框（框内背景为白色）
	setfillcolor(WHITE);           // 框内填充白色
	setlinecolor(borderColor);      // 边框颜色
	fillrectangle(x, y, x + boxWidth, y + boxHeight);


	recordBox(box.Index, x, y, x + boxWidth, y + boxHeight);
	// ★★★ 关键修正：设置文字背景为白色 ★★★
	setbkcolor(WHITE);  // 设置文字背景色为白色，与填充框一致

	// 绘制文字（现在文字背景会是白色）
	outtextxy(x + padding, y + padding, text);

	// 恢复原来的背景色，避免影响后续绘制
	setbkcolor(oldBkColor);
	box.Index++;
	if (box.Index == 4)
		box.Index = 0;
}
////////设置

struct Setsfaction {
	int StartXP[8] = { 0 };   // 增加到8个按钮
	int StartYP[8] = { 0 };
	int EndXP[8] = { 0 };
	int EndYP[8] = { 0 };
	int IndexP = 0;
} setsfaction;

// 滑动条结构体
struct Slider {
	int x, y;              // 滑动条的位置
	int width;             // 滑动条的宽度
	int height;            // 滑动条的高度
	int minValue;          // 最小值
	int maxValue;          // 最大值
	int currentValue;      // 当前值
	bool isDragging;       // 是否正在拖动
	bool isVertical;       // 是否是竖直方向
	int handlePos;         // 滑块的位置（水平为X，竖直为Y）
	int handleSize;        // 滑块的大小（水平为宽，竖直为高）

	Slider(int _x, int _y, int _w, int _h, int _min, int _max, int _default, bool _vertical = false) {
		x = _x;
		y = _y;
		width = _w;
		height = _h;
		minValue = _min;
		maxValue = _max;
		currentValue = _default;
		isDragging = false;
		isVertical = _vertical;
		handleSize = 20;
		updateHandlePos();
	}

	void updateHandlePos() {
		float percentage = (float)(currentValue - minValue) / (maxValue - minValue);

		if (isVertical) {
			handlePos = y + (int)(percentage * (height - handleSize));
		}
		else {
			handlePos = x + (int)(percentage * (width - handleSize));
		}
	}

	void updateValueFrommouse(int mouseX, int mouseY) {
		if (isVertical) {
			int clampedmouseY = mouseY;
			if (clampedmouseY < y) clampedmouseY = y;
			if (clampedmouseY > y + height) clampedmouseY = y + height;

			float percentage = (float)(clampedmouseY - y) / height;
			currentValue = minValue + (int)(percentage * (maxValue - minValue));
			handlePos = y + (int)(percentage * (height - handleSize));
		}
		else {
			int clampedmouseX = mouseX;
			if (clampedmouseX < x) clampedmouseX = x;
			if (clampedmouseX > x + width) clampedmouseX = x + width;

			float percentage = (float)(clampedmouseX - x) / width;
			currentValue = minValue + (int)(percentage * (maxValue - minValue));
			handlePos = x + (int)(percentage * (width - handleSize));
		}
	}

	void draw() {
		if (isVertical) {
			// 绘制竖直滑动条背景
			setfillcolor(RGB(200, 200, 200));
			setlinecolor(RGB(100, 100, 100));
			fillrectangle(x, y, x + width, y + height);

			// 绘制滑块
			if (isDragging) {
				setfillcolor(RGB(100, 150, 255));
			}
			else {
				setfillcolor(RGB(150, 150, 255));
			}
			fillrectangle(x - 5, handlePos, x + width + 5, handlePos + handleSize);

			// 显示当前值
			TCHAR valueStr[20];
			_stprintf_s(valueStr, _T("%d"), currentValue);
			settextcolor(BLACK);
			settextstyle(20, 0, _T("Arial"));
			outtextxy(x + width + 15, handlePos, valueStr);
		}
		else {
			// 绘制水平滑动条背景
			setfillcolor(RGB(200, 200, 200));
			setlinecolor(RGB(100, 100, 100));
			fillrectangle(x, y, x + width, y + height);

			// 绘制滑块
			if (isDragging) {
				setfillcolor(RGB(100, 150, 255));
			}
			else {
				setfillcolor(RGB(150, 150, 255));
			}
			fillrectangle(handlePos, y - 5, handlePos + handleSize, y + height + 5);

			// 显示当前值
			TCHAR valueStr[20];
			_stprintf_s(valueStr, _T("%d"), currentValue);
			settextcolor(BLACK);
			settextstyle(20, 0, _T("Arial"));
			outtextxy(x + width + 10, y, valueStr);
		}
	}

	bool ismouseOnHandle(int mouseX, int mouseY) {
		if (isVertical) {
			return mouseX >= x - 5 && mouseX <= x + width + 5 &&
				mouseY >= handlePos && mouseY <= handlePos + handleSize;
		}
		else {
			return mouseX >= handlePos && mouseX <= handlePos + handleSize &&
				mouseY >= y - 5 && mouseY <= y + height + 5;
		}
	}

	bool ismouseOnSlider(int mouseX, int mouseY) {
		if (isVertical) {
			return mouseX >= x && mouseX <= x + width &&
				mouseY >= y && mouseY <= y + height;
		}
		else {
			return mouseX >= x && mouseX <= x + width &&
				mouseY >= y && mouseY <= y + height;
		}
	}
};

void recordass(int IndexP, int x1, int y1, int x2, int y2)
{
	setsfaction.StartXP[IndexP] = x1;
	setsfaction.StartYP[IndexP] = y1;
	setsfaction.EndXP[IndexP] = x2;
	setsfaction.EndYP[IndexP] = y2;
}

void BoxText(int x, int y, LPCTSTR text, int fontSize,
	COLORREF textColor, COLORREF borderColor, int padding)
{
	COLORREF oldBkColor = getbkcolor();

	settextstyle(fontSize, 0, _T("楷体"));
	settextcolor(textColor);

	int textWidth = textwidth(text);
	int textHeight = textheight(text);

	int boxWidth = textWidth + padding * 2;
	int boxHeight = textHeight + padding * 2;

	setfillcolor(WHITE);
	setlinecolor(borderColor);
	fillrectangle(x, y, x + boxWidth, y + boxHeight);

	recordass(setsfaction.IndexP, x, y, x + boxWidth, y + boxHeight);
	setbkcolor(WHITE);
	outtextxy(x + padding, y + padding, text);

	setbkcolor(oldBkColor);
	setsfaction.IndexP++;
	if (setsfaction.IndexP == 8)  // 增加到8
		setsfaction.IndexP = 0;
}
// 创建三个竖直滑动条分别控制RGB
Slider redSlider(30, 100, 20, 300, 0, 255, 200, true);     // 红色分量
Slider greenSlider(80, 100, 20, 300, 0, 255, 231, true);   // 绿色分量
Slider blueSlider(130, 100, 20, 300, 0, 255, 232, true);   // 蓝色分量

// 为每个设置项创建水平滑动条 - 调整起始Y坐标和间距
int sliderStartYP = 50;  // 起始Y坐标上移
int sliderStepY = 55;   // 间距缩小一点以容纳更多滑动条

// 基础设置（8个）
Slider bulletDamageSlider(500, sliderStartYP, 200, 20, 0, 1, 0, false);                    // 子弹帧伤
Slider autoFireSlider(500, sliderStartYP + sliderStepY * 1, 200, 20, 0, 1, 0, false);      // 自动射击开关
Slider autoFireRateSlider(500, sliderStartYP + sliderStepY * 2, 200, 20, 1, 10, 5, false); // 自动射击速率
Slider bulletPenetrationSlider(500, sliderStartYP + sliderStepY * 3, 200, 20, 1, 10, 1, false); // 子弹穿透
Slider featureModeSlider(500, sliderStartYP + sliderStepY * 4, 200, 20, 1, 4, 1, false);   // 特色模式（原难度系数）
Slider meteorSpeedSlider(500, sliderStartYP + sliderStepY * 5, 200, 20, 1, 20, 5, false); // 陨石生成速度
Slider bulletDamagePowerSlider(500, sliderStartYP + sliderStepY * 6, 200, 20, 1, 50, 1, false); // 子弹伤害
Slider meteorHealthSlider(500, sliderStartYP + sliderStepY * 7, 200, 20, 1, 100, 10, false); // 陨石血量
Slider meteorMoveSpeedSlider(500, sliderStartYP + sliderStepY * 8, 200, 20, 1, 15, 2, false); // 陨石移动速度

Slider* currentDraggingSlider = nullptr;
////////设置
int main()
{
	initgraph(640, 815, EX_SHOWCONSOLE);
	setbkcolor(RGB(redSlider.currentValue, greenSlider.currentValue, blueSlider.currentValue));
	cleardevice();

	// 初始化时间变量
	DWORD lastTime = GetTickCount();  // 上一次的时间
	DWORD currentTime;                // 当前时间
	DWORD deltaTime;                  // 时间差（毫秒）

	TCHAR str[200];
	TCHAR Rainstone_num[200];
	TCHAR Bullet[200];
	TCHAR Banzai[200];
	int Jugban = 0;
	int Bullet_num = 0;
	int score = 0;
	int Distroy_NUM = 0;
	setbkmode(TRANSPARENT);

	// 使用当前时间作为种子
	std::mt19937 rng(time(0));

	// 定义分布范围：1 到 815
	std::uniform_int_distribution<int> dist(1, 815);

	// 生成随机数
	int randomNum;

	// banzai图案设定
	int squareHalfSide = 30;  // 正方形半边长（原外圆半径）
	int innerRadius = 15;      // 内圆半径（保持不变）

	// 定义偏移量
	int offsetX = 32;
	int offsetY = 32;

	// 加载所有图像（只执行一次）
	loadimage(&img_background, _T("./assets/neoBackground3G.png"), 640, 815);
	loadimage(&rainstone, _T("./assets/rainstone.png"), 64, 64);
	loadimage(&rainstone_mask, _T("./assets/rainstone_MG.png"), 64, 64);
	loadimage(&lineplain, _T("./assets/lineplain.png"), 64, 64);
	loadimage(&lineplain_mask, _T("./assets/lineplainmask.png"), 64, 64);
	loadimage(&BULLET, _T("./assets/BUTTET.png"), 20, 60);
	loadimage(&BULLET_mask, _T("./assets/BUTTET_B.png"), 20, 60);
	loadimage(&banzaiplain, _T("./assets/banzai_plain.png"), 64, 64);

	//加载陨石
	////////////////
	bool timehit = 0;
	bool Autoshoot = 0;
	float inautoshoot = 1.0f / 5.0f;
	int throughtimes = 1;

	int specialpoint = 1;

	float becomestone= 1.0f / 3.0f;  // 1/3 ≈ 0.333秒生成一个
	int Bulletdamge = 1;
	float RainSpeed = 2;
	int rainblood = 10;
	/////////////
	float stonetimer = 0;
	float Autotimer = 0;
	for (int i = 0; i < STONE_NUM; i++)//
	{
		rain[i].x = 0;
		rain[i].y = 0;
		rain[i].blood = 10;
		rain[i].live = false;
	}
	

	int Menupoint = 1;//Menu控制
	while (1)
	{
		BeginBatchDraw();
		cleardevice();
		currentTime = GetTickCount();

		// 计算时间差（距离上一帧过去了多少毫秒）
		deltaTime = currentTime - lastTime;

		// 如果时间差太小，就等待一下（控制帧率）
		if (deltaTime < 16)  // 60 FPS ≈ 每帧16.67毫秒
		{
			Sleep(16 - deltaTime);  // 等待不足的时间

		}
		// 更新上一帧时间
		lastTime = currentTime;
		switch (Menupoint) {
		case 0://游戏循环
		{
			// ★★★ 从滑动条获取当前设置值 ★★★
	// 子弹帧伤（开关）
			timehit = bulletDamageSlider.currentValue;

			// 自动射击开关
			Autoshoot = autoFireSlider.currentValue;

			// 自动射击速率
			if (autoFireRateSlider.currentValue > 0)
				inautoshoot = 1.0f / autoFireRateSlider.currentValue;
			else
				inautoshoot = 1.0f;  // 防止除以0

			// 子弹穿透次数
			throughtimes = bulletPenetrationSlider.currentValue;

			// 特色模式
			specialpoint = featureModeSlider.currentValue;

			// 陨石生成速度
			if (meteorSpeedSlider.currentValue > 0)
				becomestone = 1.0f / meteorSpeedSlider.currentValue;
			else
				becomestone = 1.0f;  // 防止除以0

			// 子弹伤害
			Bulletdamge = bulletDamagePowerSlider.currentValue;

			// 陨石血量
			rainblood = meteorHealthSlider.currentValue;

			// 陨石移动速度
			RainSpeed = (float)meteorMoveSpeedSlider.currentValue;
			stonetimer += 1.0f / 60.0f;//一帧加1/60秒
			if (specialpoint == 2)//野兽模式
			{
				becomestone = 0.5;
				rainblood = 50;
				RainSpeed = 1;
				Bulletdamge = 2;
			}
			else if (specialpoint == 3)//闪电模式
			{
				becomestone = 0.1;
				rainblood = 4;
				RainSpeed = 8;
				Bulletdamge = 1;
			}
			else if (specialpoint == 4)//虫巢
			{
				becomestone = 1.0 / 30.0;
				rainblood = 2;
				RainSpeed = 2.25;
				Bulletdamge = 1;
			}
			//特色模式
			if (stonetimer>=becomestone)
			{
				for (int i = 0; i < STONE_NUM; i++)
				{
					if (!rain[i].live)
					{
						randomNum = dist(rng);
						rain[i].x = randomNum;

						rain[i].y = 0;
						rain[i].live = true;
						//产生了一个陨石 退出循环
						rain[i].blood = rainblood;
						break;
					}
				}
				stonetimer = 0;
			}

			//背景
			putimage(0, 0, &img_background);
			_stprintf_s(str, _T("score:%d"), score++);
			_stprintf_s(Rainstone_num, _T("Dtory:%d"), Distroy_NUM);
			_stprintf_s(Bullet, _T("Shoot:%d"), Bullet_num);
			_stprintf_s(Banzai, _T("天脑海卡_板载！:%d"), Jugban);
			outtextxy(30, 50, str);
			outtextxy(400, 50, Rainstone_num);
			Autotimer += 1.0f / 60.0f;//auto shooting
			while (peekmessage(&msg, EX_MOUSE | EX_KEY))
			{
				if (msg.message == WM_LBUTTONDOWN && Autoshoot==0)
				{

					for (int i = 0; i < BULLET_NUM; i++)
					{
						if (!bull[i].live)
						{
							bull[i].x = Mouse.x + 22;
							bull[i].y = Mouse.y - 30;
							bull[i].live = true;
							Bullet_num++;
							bull[i].Bullettime =throughtimes;
							//产生了一个子弹 退出循环
							break;
						}
					}
				}
				//鼠标移动
				else if (msg.message == WM_MOUSEMOVE && Mouse.live == 1)
				{
					Mouse.x = msg.x;
					Mouse.y = msg.y;
				}
				else if (msg.message == WM_RBUTTONDOWN && Mouse.banzaitime == 1000)
				{
					Mouse.banzai = 1;
					Mouse.banzaitime = 0;
				}
				if (msg.vkcode == VK_ESCAPE) {  // ESC 键
					Menupoint = 1;  // 返回菜单
				}
			}
			if (Autoshoot == 1&&Autotimer>=inautoshoot)
			{
				for (int i = 0; i < BULLET_NUM; i++)
				{
					if (!bull[i].live)
					{
						bull[i].x = Mouse.x + 22;
						bull[i].y = Mouse.y - 30;
						bull[i].live = true;
						Bullet_num++;
						bull[i].Bullettime = throughtimes;
						//产生了一个子弹 退出循环
						break;
					}
				}
				Autotimer = 0;
			}
			//绘制飞机
			putimage(Mouse.x, Mouse.y, &lineplain, SRCINVERT);
			putimage(Mouse.x, Mouse.y, &lineplain_mask, NOTSRCERASE);
			//板载功能冷却
			if (Mouse.banzaitime < 1000)
				Mouse.banzaitime = Mouse.banzaitime + 1;
			else//激活画面
			{
				// 计算偏移后的中心点坐标
				int drawCenterX = Mouse.x + offsetX;
				int drawCenterY = Mouse.y + offsetY;

				// 绘制绿色正方形边框
				setlinecolor(GREEN);
				setfillcolor(GREEN);
				// 绘制空心正方形（只画边框）- 使用偏移后的中心点
				rectangle(drawCenterX - squareHalfSide, drawCenterY - squareHalfSide,
					drawCenterX + squareHalfSide, drawCenterY + squareHalfSide);

				// 绘制12等分红色分割线
				setlinecolor(RED);

				// 12等分，每份角度为 360/12 = 30度 = PI/6 弧度
				for (int i = 0; i < 12; i++)
				{
					double angle = i * (2 * PI / 12);  // 计算当前角度（弧度）

					// 计算内圆上的起点坐标 - 使用偏移后的中心点
					int beginX = drawCenterX + (int)(innerRadius * cos(angle));
					int beginY = drawCenterY + (int)(innerRadius * sin(angle));

					// 计算与正方形边界的交点 - 使用偏移后的中心点
					int endX, endY;
					getSquareIntersection(drawCenterX, drawCenterY, squareHalfSide, angle, &endX, &endY);

					// 从内圆边缘画线到正方形边界
					line(beginX, beginY, endX, endY);
				}

				// 绘制中心红色实心圆 - 使用偏移后的中心点
				setfillcolor(RED);
				setlinecolor(RED);  // 设置边框也为红色，看起来更统一
				fillcircle(drawCenterX, drawCenterY, innerRadius);

			}
			/////////
			for (int i = 0; i < STONE_NUM; i++)
			{
				if (!rain[i].live) continue;// 跳过不活动的陨石
				//碰到子弹扣血
				for (int j = 0; j < BULLET_NUM; j++)
				{
					if (!bull[j].live) continue;  // 跳过不活动的子弹
					if (bull[j].x - rain[i].x <= 64 && rain[i].x - bull[j].x <= 20 && bull[j].y - rain[i].y <= 64)//碰撞检测
					{
						
							
						if (bull[j].Bulletlast[i] == 0)  // 这一帧第一次碰到这个陨石
						{
							rain[i].blood=rain[i].blood-Bulletdamge;              // 陨石扣血
							bull[j].Bulletlast[i] = 1;    // 标记已碰撞

							bull[j].Bullettime--;         // 减少穿透次数

							if (bull[j].Bullettime <= 0)
							{
								bull[j].live = false;
								//  也要重置
								for (int k = 0; k < STONE_NUM; k++)
								{
									bull[j].Bulletlast[k] = 0;
								}
							}
						}
						if (timehit == 1)
							bull[j].live = true;
					}
					//如果陨石血量为0，陨石也消失
					if (rain[i].blood <= 0)
					{
						rain[i].live = false;
						score = score + 500;
						Distroy_NUM++;
						break;  // 陨石已消失，跳出内层循环
					}
				}
			}
			for (int i = 0; i < STONE_NUM; i++)
			{
				if (rain[i].live)
				{
					//陨石图片
					putimage(rain[i].x, rain[i].y, &rainstone_mask, NOTSRCERASE);
					putimage(rain[i].x, rain[i].y, &rainstone, SRCINVERT);
				}
				rain[i].y += RainSpeed;
				if (rain[i].y > 880)
				{
					rain[i].live = false;
				}

			}
			//绘制子弹
			for (int i = 0; i < BULLET_NUM; i++)
			{
				if (bull[i].live)
				{
					//子丹图片

					putimage(bull[i].x, bull[i].y, &BULLET, SRCINVERT);
					putimage(bull[i].x, bull[i].y, &BULLET_mask, NOTSRCERASE);

					bull[i].y -= 6;
					if (bull[i].y < -30)
					{
						bull[i].live = false;/////////////////////////////////////
						for (int j = 0; j < STONE_NUM; j++)
						{
							bull[i].Bulletlast[j] = 0;//穿透归零
						}
					}
				}

			}



			//陨石撞击逻辑


			//击落飞机
			for (int i = 0; i < STONE_NUM; i++)
			{
				if ((rain[i].y - Mouse.y <= 57 && rain[i].y - Mouse.y >= -57)&& (rain[i].y - Mouse.y >= 32 && rain[i].y - Mouse.y <= -32)&&
					(rain[i].x - Mouse.x <= 57 && rain[i].x - Mouse.x >= -57)&& (rain[i].x - Mouse.x >= 32 && rain[i].x - Mouse.x <= -32))
				{
					if ((rain[i].x - (Mouse.x - 32)) * (rain[i].x - (Mouse.x - 32)) + (rain[i].y - (Mouse.y - 32)) * (rain[i].y - (Mouse.y - 32)) <= 625 ||
						(rain[i].x - (Mouse.x + 32)) * (rain[i].x - (Mouse.x + 32)) + (rain[i].y - (Mouse.y + 32)) * (rain[i].y - (Mouse.y + 32)) <= 625 ||
						(rain[i].x - (Mouse.x - 32)) * (rain[i].x - (Mouse.x - 32)) + (rain[i].y - (Mouse.y + 32)) * (rain[i].y - (Mouse.y + 32)) <= 625 ||
						(rain[i].x - (Mouse.x + 32)) * (rain[i].x - (Mouse.x + 32)) + (rain[i].y - (Mouse.y - 32)) * (rain[i].y - (Mouse.y - 32)) <= 625 
						)
					{
						if ((rain[i].y - Mouse.y <= 57 && rain[i].y - Mouse.y >= -57) && (rain[i].x - Mouse.x <= 57 && rain[i].x - Mouse.x >= -57))
						{

							if (Mouse.banzai == 0 && rain[i].live == 1)//优化：结束画面简单做一个
							{
								for (int i = 0; i < STONE_NUM; i++) rain[i].live = false;
								for (int i = 0; i < BULLET_NUM; i++) bull[i].live = false;
								Mouse.banzai = 0;
								Mouse.banzaitime = 0;
								Menupoint = 2;
								break;
							}

							else if (Mouse.banzai == 1 && rain[i].live == 1)
							{

								for (int k = 0; k < STONE_NUM; k++)
								{

									if ((rain[k].y - Mouse.y <= 256 && rain[k].y - Mouse.y >= -256) && (rain[k].x - Mouse.x <= 256 && rain[k].x - Mouse.x >= -256))
									{
										rain[k].live = false;
										Distroy_NUM++;
										score = score + 500;
									}
								}
								for (int k = 0; k < BULLET_NUM; k++)
									if ((bull[k].y - Mouse.y <= 256 && bull[k].y - Mouse.y >= -256) && (bull[k].x - Mouse.x <= 256 && bull[k].x - Mouse.x >= -256))
									{
										bull[k].live = false;
									}
								Jugban++;
								Mouse.banzai = false;
							}

						}
					}
				}
				else
				{
					if ((rain[i].y - Mouse.y <= 57 && rain[i].y - Mouse.y >= -57) && (rain[i].x - Mouse.x <= 57 && rain[i].x - Mouse.x >= -57))
					{

						if (Mouse.banzai == 0 && rain[i].live == 1)
						{
							for (int i = 0; i < STONE_NUM; i++) rain[i].live = false;
							for (int i = 0; i < BULLET_NUM; i++) bull[i].live = false;
							Mouse.banzai = 0;
							Mouse.banzaitime = 0;
							for (int i = 0; i < BULLET_NUM; i++)
							{
								bull[i].live = false;
								//  重置所有子弹的碰撞标记
								for (int j = 0; j < STONE_NUM; j++)
								{
									bull[i].Bulletlast[j] = 0;
								}
							}
							Menupoint = 2;
							break;
						}

						else if (Mouse.banzai == 1 && rain[i].live == 1)
						{

							for (int k = 0; k < STONE_NUM; k++)
							{

								if ((rain[k].y - Mouse.y <= 256 && rain[k].y - Mouse.y >= -256) && (rain[k].x - Mouse.x <= 256 && rain[k].x - Mouse.x >= -256))
								{
									rain[k].live = false;
									Distroy_NUM++;
									score = score + 500;
								}
							}
							for (int k = 0; k < BULLET_NUM; k++)
								if ((bull[k].y - Mouse.y <= 256 && bull[k].y - Mouse.y >= -256) && (bull[k].x - Mouse.x <= 256 && bull[k].x - Mouse.x >= -256))
								{
									bull[k].live = false;
								}
							Jugban++;
							Mouse.banzai = false;
						}

					}
				}

			}
			break;
		}
		case 1://优化点，菜单只需要画一遍就行了
		{
			// 菜单界面
			Resize(NULL, 640, 815);  // NULL 表示调整当前绘图窗口
			setbkcolor(RGB(redSlider.currentValue, greenSlider.currentValue, blueSlider.currentValue));
			cleardevice();

			// 处理鼠标消息

			while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {
				if (msg.message == WM_MOUSEMOVE) {
					Mouse.x = msg.x;
					Mouse.y = msg.y;
				}

				if (msg.message == WM_LBUTTONDOWN)
				{
					if (Mouse.x >= box.startX[0] && Mouse.x <= box.endX[0] && Mouse.y >= box.startY[0] && Mouse.y <= box.endY[0]) {
						Menupoint = 0;
					}
					else if (Mouse.x >= box.startX[1] && Mouse.x <= box.endX[1] && Mouse.y >= box.startY[1] && Mouse.y <= box.endY[1]) {
						Menupoint = 5;
					}
					else if (Mouse.x >= box.startX[2] && Mouse.x <= box.endX[2] && Mouse.y >= box.startY[2] && Mouse.y <= box.endY[2]) {
						Menupoint = 3;
					}
					else if (Mouse.x >= box.startX[3] && Mouse.x <= box.endX[3] && Mouse.y >= box.startY[3] && Mouse.y <= box.endY[3]) {
						Menupoint = 4;
					}
				}
			}
			drawTextWithBox(200, 100, _T("开始游戏"), 50, RGB(0, 150, 0), RGB(0, 150, 0), 15);
			drawTextWithBox(200, 200, _T("设置"), 40, RGB(0, 0, 255), RGB(0, 0, 255), 12);
			drawTextWithBox(200, 280, _T("帮助"), 35, RGB(255, 0, 0), RGB(255, 0, 0), 10);
			drawTextWithBox(200, 350, _T("退出游戏"), 45, RGB(128, 128, 128), RGB(128, 128, 128), 15);
			break;
		}
		case 2:
		{
			//结束动画
			cleardevice();  // 清屏
			putimage(0, 0, &img_background);

			settextcolor(GREEN);
			settextstyle(30, 0, _T("宋体"));
			outtextxy(260, 200, _T("Game Over！"));
			//分数，击杀数，射击，技能
			outtextxy(410, 350, str);
			outtextxy(100, 350, Rainstone_num);
			outtextxy(260, 350, Bullet);
			outtextxy(200, 600, Banzai);

			putimage(160-32, 400, &rainstone_mask, NOTSRCERASE);
			putimage(160-32, 400, &rainstone, SRCINVERT);

			putimage(320-10, 400, &BULLET, SRCINVERT);
			putimage(320-10, 400, &BULLET_mask, NOTSRCERASE);

			putimage(480-32, 400, &lineplain, SRCINVERT);
			putimage(480-32, 400, &lineplain_mask, NOTSRCERASE);

			putimage(320-32,500, &banzaiplain);

			while (peekmessage(&msg, EX_KEY))
			{
				if (msg.vkcode == VK_ESCAPE) {  // ESC 键
					// ★★★ 初始化所有游戏状态 ★★★
					score = 0;                    // 分数归零
					Distroy_NUM = 0;               // 击杀数归零
					Bullet_num = 0;                 // 射击数归零
					Jugban = 0;                     // 技能次数归零

					// 重置所有陨石
					for (int i = 0; i < STONE_NUM; i++) {
						rain[i].live = false;
						rain[i].blood = 10;
					}

					// 重置所有子弹
					for (int i = 0; i < BULLET_NUM; i++) {
						bull[i].live = false;
						// 重置所有子弹的碰撞标记
						for (int j = 0; j < STONE_NUM; j++) {
							bull[i].Bulletlast[j] = 0;
						}
					}

					// 重置鼠标/飞机状态
					Mouse.x = 400;                  // 重置飞机位置
					Mouse.y = 600;
					Mouse.banzai = 0;                // 板载状态重置
					Mouse.banzaitime = 0;             // 板载冷却重置

					// 重置计时器
					stonetimer = 0;
					Autotimer = 0;

					Menupoint = 1;  // 返回菜单
				}

			}
			break;
		}
		case 3:
		{

			cleardevice();  // 清屏
			putimage(0, 0, &img_background);

			settextcolor(GREEN);
			settextstyle(30, 0, _T("宋体"));
			outtextxy(20, 200, _T("左键进入"));
			outtextxy(20, 250, _T("短按esc回到菜单，游戏中esc暂停"));
			outtextxy(20, 300, _T("游戏左键发射子弹，等机身有红日图案时"));
			outtextxy(20, 350, _T("右键板载清除陨石"));

			putimage(160 - 32, 400, &rainstone_mask, NOTSRCERASE);
			putimage(160 - 32, 400, &rainstone, SRCINVERT);

			putimage(320 - 10, 400, &BULLET, SRCINVERT);
			putimage(320 - 10, 400, &BULLET_mask, NOTSRCERASE);

			putimage(480 - 32, 400, &lineplain, SRCINVERT);
			putimage(480 - 32, 400, &lineplain_mask, NOTSRCERASE);
			putimage(320 - 32, 500, &banzaiplain);
			while (peekmessage(&msg, EX_KEY))
			{
				if (msg.vkcode == VK_ESCAPE) {  // ESC 键
					Menupoint = 1;  // 返回菜单
				}

			}
			break;
		}
		case 5:
		{
			// 使用三个滑动条的值合成背景色
			setbkcolor(RGB(redSlider.currentValue, greenSlider.currentValue, blueSlider.currentValue));
			Resize(NULL, 800, 600);  // NULL 表示调整当前绘图窗口

			// 绘制三个竖直背景色滑动条
			redSlider.draw();
			greenSlider.draw();
			blueSlider.draw();

			// 显示提示文字
			settextcolor(BLACK);
			settextstyle(20, 0, _T("楷体"));
			outtextxy(30, 50, _T("R"));
			outtextxy(80, 50, _T("G"));
			outtextxy(130, 50, _T("B"));

			// 显示当前RGB值
			TCHAR rgbStr[100];
			_stprintf_s(rgbStr, _T("R:%d G:%d B:%d"),
				redSlider.currentValue,
				greenSlider.currentValue,
				blueSlider.currentValue);
			outtextxy(30, 420, rgbStr);

			// 绘制文本框 - 调整起始Y坐标和间距
			int StartYP = 40;    // 起始Y坐标
			int stepY = 52;      // 间距

			BoxText(260, StartYP, _T("子弹帧伤;"), 28, RGB(255, 0, 0), RGB(128, 0, 128), 11);
			BoxText(260, StartYP + stepY * 1 + 10, _T("自动射击;"), 28, RGB(255, 165, 0), RGB(255, 0, 0), 11);
			BoxText(260, StartYP + stepY * 2 + 10, _T("自动射击速率;"), 28, RGB(255, 165, 0), RGB(255, 0, 0), 11);
			BoxText(260, StartYP + stepY * 3 + 10, _T("子弹穿透;"), 28, RGB(225, 225, 0), RGB(255, 165, 0), 11);
			BoxText(260, StartYP + stepY * 4 + 10, _T("特色模式;"), 28, RGB(0, 225, 0), RGB(225, 225, 0), 11);
			BoxText(260, StartYP + stepY * 5 + 10, _T("陨石生成速度;"), 28, RGB(0, 230, 230), RGB(0, 225, 0), 11);
			BoxText(260, StartYP + stepY * 6 + 10, _T("子弹伤害;"), 28, RGB(0, 0, 255), RGB(0, 230, 230), 11);
			BoxText(260, StartYP + stepY * 7 + 10, _T("陨石血量;"), 28, RGB(128, 0, 128), RGB(0, 0, 255), 11);
			BoxText(260, StartYP + stepY * 8 + 10, _T("陨石移动速度;"), 28, RGB(255, 0, 0), RGB(128, 0, 128), 11);

			// 绘制各个设置项的滑动条
			bulletDamageSlider.draw();
			autoFireSlider.draw();
			autoFireRateSlider.draw();
			bulletPenetrationSlider.draw();
			featureModeSlider.draw();
			meteorSpeedSlider.draw();
			bulletDamagePowerSlider.draw();
			meteorHealthSlider.draw();
			meteorMoveSpeedSlider.draw();

			// 显示当前设置值
			settextcolor(BLACK);
			settextstyle(18, 0, _T("Arial"));
			outtextxy(10, 540, _T("当前设置值:"));

			TCHAR setsfactionStr[350];
			_stprintf_s(setsfactionStr, _T("子弹帧伤:%s 自动射击:%s 自动速率:%d 子弹穿透:%d 特色:%d 陨石速度:%d 子弹伤害:%d 陨石血量:%d 陨石移动速度:%d"),
				bulletDamageSlider.currentValue ? _T("开") : _T("关"),
				autoFireSlider.currentValue ? _T("开") : _T("关"),
				autoFireRateSlider.currentValue,
				bulletPenetrationSlider.currentValue,
				featureModeSlider.currentValue,
				meteorSpeedSlider.currentValue,
				bulletDamagePowerSlider.currentValue,
				meteorHealthSlider.currentValue,
				meteorMoveSpeedSlider.currentValue);
			outtextxy(10, 565, setsfactionStr);
			///
			while (peekmessage(&msg, EX_KEY | EX_MOUSE))
			{
				if (msg.vkcode == VK_ESCAPE) {  // ESC 键
					Menupoint = 1;  // 返回菜单
				}
				if (msg.message == WM_LBUTTONDOWN)
				{
					// 检查三个RGB滑动条
					Slider* rgbSliders[] = { &redSlider, &greenSlider, &blueSlider };
					for (Slider* slider : rgbSliders) {
						if (slider->ismouseOnHandle(mouse.x, mouse.y) ||
							slider->ismouseOnSlider(mouse.x, mouse.y)) {
							slider->isDragging = true;
							currentDraggingSlider = slider;
							break;
						}
					}

					// 检查各个设置项滑动条的滑块
					Slider* sliders[] = {
						&bulletDamageSlider, &autoFireSlider, &autoFireRateSlider,
						&bulletPenetrationSlider, &featureModeSlider, &meteorSpeedSlider,
						&bulletDamagePowerSlider, &meteorHealthSlider, &meteorMoveSpeedSlider
					};

					for (Slider* slider : sliders) {
						if (slider->ismouseOnHandle(mouse.x, mouse.y)) {
							slider->isDragging = true;
							currentDraggingSlider = slider;
							break;
						}
					}
				}

				if (msg.message == WM_LBUTTONUP)
				{
					if (currentDraggingSlider) {
						currentDraggingSlider->isDragging = false;
						currentDraggingSlider = nullptr;
					}
				}

				if (msg.message == WM_MOUSEMOVE)
				{
					mouse.x = msg.x;
					mouse.y = msg.y;

					if (currentDraggingSlider && currentDraggingSlider->isDragging) {
						currentDraggingSlider->updateValueFrommouse(mouse.x, mouse.y);
					}
				}
			
			}
		}
		break;
		default:
			break;
		}
		EndBatchDraw();
		Sleep(2);
		if (Menupoint == 4)
			break;
	}
	return 0;
}
