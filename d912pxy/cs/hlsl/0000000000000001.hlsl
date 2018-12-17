#define threadBlockSize 32

struct ipar {
	uint limit;
};

RWByteAddressBuffer dstBuffer : register(u0); 
RWByteAddressBuffer srcBuffer : register(u1);
RWByteAddressBuffer parBuffer : register(u2);
ConstantBuffer<ipar> par: register(b0);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	uint index = ((groupId.x * threadBlockSize) + groupIndex)*3*4;
	
	if (index >= par.limit)
		return;

	uint3 tmp = parBuffer.Load3(index);
	
	uint srcOffset = tmp.x;
	uint size = tmp.y;	
	uint offset = tmp.z;
		
	uint i = 0;
	while (i != size)
	{
		dstBuffer.Store4(offset+i,srcBuffer.Load4(srcOffset+i));
		i = i+16;
	}
}