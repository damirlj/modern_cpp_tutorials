package com.example.practice_kotlin.design_patterns.builder


class CarConfiguration2 (
    // mandatory
    var id: Int,
    var brand: String,
    var engine: Engine,
    // optionals
    var camera: Camera? = null,
    var smartphone: Smartphone? = null,
    var adas: ADAS? = null
) {


    // For aggregate updates - initialize with previous configuration (copy-constructor )
    constructor (car: CarConfiguration2) : this(car.id, car.brand, car.engine) {
        camera = car.camera
        smartphone = car.smartphone
        adas = car.adas
    }

   // For aggregate updates - the new (partial) configuration that will be applied on top of the current configuration
    fun update(car: CarConfiguration2) = apply {
        id = car.id
        brand = car.brand
        engine = car.engine

        car.camera?.let { camera = it }
        car.smartphone?.let { smartphone = it }
        car.adas?.let { adas = it }
    }


    companion object {
        /**
         * Helper method, for building the outer class, as wrapper around the apply()
         *
         * @param block This is lambda that extends the receiver - instance of the outer class,
         * which means the callable can access any non-private (public) properties of the
         * receiver (all of them), in order to proper configure the outer class
         */

        inline fun build(
            id: Int,
            brand: String,
            engine: Engine,
            block: CarConfiguration2.() -> Unit
        ) =
            CarConfiguration2(id, brand, engine).apply(block)

        // For aggregate updates
        inline fun build(currConfig: CarConfiguration2, block: CarConfiguration2.() -> Unit) =
            CarConfiguration2(currConfig).apply(block)

        fun build(currConfig: CarConfiguration2, newConfig: CarConfiguration2) =
            CarConfiguration2(currConfig).update(newConfig)
    }


    override fun toString(): String {
        return "[id=$id, brand=$brand, engine=$engine, camera=$camera, smartphone=$smartphone, adas=$adas]"
    }
}


