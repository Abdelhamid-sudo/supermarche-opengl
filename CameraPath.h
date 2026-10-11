#ifndef CAMERA_PATH_H
#define CAMERA_PATH_H

#include <vector>
#include <cmath>
#include "glm/glm.hpp"

inline glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float u)
{
	float u2 = u * u;
	float u3 = u2 * u;
	return 0.5f * ((2.0f * p1) +
				   (-p0 + p2) * u +
				   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u2 +
				   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u3);
}

class CameraPath
{
public:
	CameraPath() : mLoop(false) {}

	void setLoop(bool loop) { mLoop = loop; }

	void addKey(float time, const glm::vec3& position, const glm::vec3& target)
	{
		Key k = { time, position, target };
		mKeys.push_back(k);
	}

	float endTime() const { return mKeys.empty() ? 0.0f : mKeys.back().time; }

	void sample(float t, glm::vec3& position, glm::vec3& target) const
	{
		int n = (int)mKeys.size();
		if (n == 0)
			return;
		if (mLoop && n > 2)
		{

			float start = mKeys[0].time, duration = mKeys[n - 1].time - start;
			t = start + fmodf(t - start, duration);
			if (t < start)
				t += duration;
		}
		if (t <= mKeys[0].time || n == 1)
		{
			position = mKeys[0].position;
			target = mKeys[0].target;
			return;
		}
		if (t >= mKeys[n - 1].time)
		{
			position = mKeys[n - 1].position;
			target = mKeys[n - 1].target;
			return;
		}

		int i = 0;
		while (mKeys[i + 1].time < t)
			i++;
		float u = (t - mKeys[i].time) / (mKeys[i + 1].time - mKeys[i].time);

		int i0 = (i > 0) ? i - 1 : (mLoop ? n - 2 : 0);
		int i3 = (i + 2 < n) ? i + 2 : (mLoop ? 1 : n - 1);
		const Key& k0 = mKeys[i0];
		const Key& k1 = mKeys[i];
		const Key& k2 = mKeys[i + 1];
		const Key& k3 = mKeys[i3];

		position = catmullRom(k0.position, k1.position, k2.position, k3.position, u);
		target = catmullRom(k0.target, k1.target, k2.target, k3.target, u);
	}

private:
	struct Key
	{
		float time;
		glm::vec3 position;
		glm::vec3 target;
	};
	std::vector<Key> mKeys;
	bool mLoop;
};

#endif
