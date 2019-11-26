#define threadBlockSize 32
#define dstBufferStride 8704
#define dataElementSz 16
#define ctlElementSz 16

RWByteAddressBuffer dstBuffer : register(u0); 
RWByteAddressBuffer srcData : register(u1);
RWByteAddressBuffer srcCtl  : register(u2);

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	uint2 shift = {dstBufferStride, 1};
	uint index = ((groupId.x * threadBlockSize) + groupIndex);
	
	//fv0
	//fv1
	//fv2
	//fv3		
	
	//dstOffset = x
	//startBatch = y 
	//endBatch = z
	//cacheAlignPlace
		
	//megai2: right now data and control elements have same size, so we can skip extra ops
	//don't get confused!
	index *= ctlElementSz;	
				
	uint4 control = srcCtl.Load4(index);
					
	if (control.y == control.z)
		return;		
		
	uint4 data = srcData.Load4(index);		
	
	control.x *= dataElementSz;		
	control.x += control.y * dstBufferStride;
	
	while (control.y != control.z)
	{
		dstBuffer.Store4(control.x, data);
		control.xy += shift;
	}
}